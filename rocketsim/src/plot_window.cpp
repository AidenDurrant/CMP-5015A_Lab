// plot_window.cpp -- GIVEN. The live window behind Plot::show(): a ground-station
// style display (phosphor green on black) that replays the flight in real time.
// Drawn with SFML 3, fetched and built automatically by CMake. Nothing in the
// labs depends on reading this file.
//
// Build without it with:  cmake -B build -DROCKETSIM_LIVE=OFF
//
// NOTE -- this file and the plotting was created with the use of AI tools. Declaring for transparency - Aiden

#include "plot.h"

#include <iostream>

#ifndef ROCKETSIM_LIVE

void Plot::show() const
{
    std::cout << "(built without the live window; open the SVG instead)\n";
}

#else

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#ifndef ROCKETSIM_ASSET_DIR
#define ROCKETSIM_ASSET_DIR "assets"
#endif

namespace
{
    const sf::Color PHOSPHOR(57, 255, 20);
    const sf::Color PHOSPHOR_SOFT(157, 255, 128);
    const sf::Color PHOSPHOR_DIM(31, 138, 14);
    const sf::Color GRID_MINOR(11, 58, 6);
    const sf::Color GRID_MAJOR(20, 107, 10);
    const sf::Color BACKGROUND(0, 0, 0);

    // Series are told apart by brightness and dash pattern, as on a
    // single-colour display.
    const sf::Color SERIES_COLOUR[] = {PHOSPHOR, PHOSPHOR_SOFT, PHOSPHOR, PHOSPHOR_SOFT,
                                       PHOSPHOR, PHOSPHOR_SOFT, PHOSPHOR, PHOSPHOR_SOFT};
    const std::vector<float> SERIES_DASH[] = {{}, {}, {12.f, 6.f}, {12.f, 6.f}, {3.f, 5.f}, {3.f, 5.f},
                                              {14.f, 4.f, 3.f, 4.f}, {14.f, 4.f, 3.f, 4.f}};
    const int STYLE_COUNT = 8;
    const float SPEEDS[] = {1.f, 2.f, 5.f, 10.f, 20.f};
    const int SPEED_COUNT = 5;

    std::string fmt(double value)
    {
        if (std::fabs(value) < 1e-9)
        {
            value = 0.0;
        }
        std::ostringstream out;
        out << std::setprecision(6) << value;
        return out.str();
    }

    std::string fixed(double value, int places)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(places) << value;
        return out.str();
    }

    std::string clock(double seconds)
    {
        const int whole = static_cast<int>(seconds);
        std::ostringstream out;
        out << "T+" << std::setfill('0') << std::setw(2) << whole / 60 << ':' << std::setw(2) << whole % 60 << '.'
            << static_cast<int>((seconds - whole) * 10);
        return out.str();
    }

    void thickSegment(sf::VertexArray& tris, sf::Vector2f a, sf::Vector2f b, float thickness, sf::Color colour)
    {
        const sf::Vector2f d = b - a;
        const float len = std::sqrt(d.x * d.x + d.y * d.y);
        if (len < 1e-6f)
        {
            return;
        }
        const sf::Vector2f n(-d.y / len * thickness / 2.f, d.x / len * thickness / 2.f);
        tris.append(sf::Vertex{a + n, colour});
        tris.append(sf::Vertex{b + n, colour});
        tris.append(sf::Vertex{a - n, colour});
        tris.append(sf::Vertex{b + n, colour});
        tris.append(sf::Vertex{b - n, colour});
        tris.append(sf::Vertex{a - n, colour});
    }

    // A polyline with an on/off dash pattern measured along its length.
    void dashedPolyline(sf::VertexArray& tris, const std::vector<sf::Vector2f>& pts,
                        const std::vector<float>& pattern, float thickness, sf::Color colour)
    {
        if (pts.size() < 2)
        {
            return;
        }
        if (pattern.empty())
        {
            for (std::size_t i = 1; i < pts.size(); ++i)
            {
                thickSegment(tris, pts[i - 1], pts[i], thickness, colour);
            }
            return;
        }
        std::size_t dash = 0;
        float phase = 0.f;
        bool on = true;
        for (std::size_t i = 1; i < pts.size(); ++i)
        {
            const sf::Vector2f a = pts[i - 1];
            const sf::Vector2f d = pts[i] - a;
            const float len = std::sqrt(d.x * d.x + d.y * d.y);
            if (len < 1e-6f)
            {
                continue;
            }
            float pos = 0.f;
            while (pos < len)
            {
                const float take = std::min(pattern[dash] - phase, len - pos);
                if (on)
                {
                    thickSegment(tris, a + d * (pos / len), a + d * ((pos + take) / len), thickness, colour);
                }
                pos += take;
                phase += take;
                if (phase >= pattern[dash] - 1e-4f)
                {
                    phase = 0.f;
                    dash = (dash + 1) % pattern.size();
                    on = !on;
                }
            }
        }
    }

    struct Labeller
    {
        const sf::Font* font = nullptr;

        // anchor: 0 = left edge at x, 1 = centred on x, 2 = right edge at x.
        void draw(sf::RenderTarget& target, const std::string& text, float x, float y, unsigned size, int anchor,
                  sf::Color colour = PHOSPHOR, float rotation = 0.f) const
        {
            if (font == nullptr)
            {
                return;
            }
            sf::Text label(*font, text, size);
            label.setFillColor(colour);
            const sf::FloatRect bounds = label.getLocalBounds();
            const float shift = anchor == 1 ? bounds.size.x / 2.f : (anchor == 2 ? bounds.size.x : 0.f);
            label.setOrigin({bounds.position.x + shift, bounds.position.y + bounds.size.y / 2.f});
            label.setPosition({x, y});
            label.setRotation(sf::degrees(rotation));
            target.draw(label);
        }
    };

    bool saveScreenshot(const sf::RenderWindow& window, const std::string& path)
    {
        sf::Texture texture;
        if (!texture.resize(window.getSize()))
        {
            return false;
        }
        texture.update(window);
        return texture.copyToImage().saveToFile(path);
    }
}

void Plot::show() const
{
    drawWindow();
}

void Plot::drawWindow() const
{
    const unsigned width = 960;
    const unsigned height = 600;
    const float left = 85.f;
    const float right = 30.f;
    const float top = 60.f;
    const float bottom = 70.f;
    const float plotW = width - left - right;
    const float plotH = height - top - bottom;

    sf::Font font;
    Labeller text;
    const std::string fontPath = std::string(ROCKETSIM_ASSET_DIR) + "/DejaVuSansMono.ttf";
    if (font.openFromFile(fontPath) || font.openFromFile(std::string(ROCKETSIM_ASSET_DIR) + "/DejaVuSans.ttf"))
    {
        text.font = &font;
    }
    else
    {
        std::cerr << "Could not load " << fontPath << "; the window will have no text\n";
    }

    sf::RenderWindow window(sf::VideoMode({width, height}), "rocketsim - " + title_);
    window.setFramerateLimit(60);

    // Scanlines: a dark line every third pixel row, drawn over everything.
    sf::VertexArray scanlines(sf::PrimitiveType::Lines);
    for (unsigned y = 0; y < height; y += 3)
    {
        scanlines.append(sf::Vertex{{0.f, static_cast<float>(y)}, sf::Color(0, 0, 0, 90)});
        scanlines.append(sf::Vertex{{static_cast<float>(width), static_cast<float>(y)}, sf::Color(0, 0, 0, 90)});
    }

    // Replay state.
    bool fit = false;                  // F toggles the fixed frame off
    int speedIndex = 0;                // +/- move through SPEEDS
    double replayTime = 0.0;           // simulated seconds shown so far
    const double total = duration();
    bool saveRequested = false;
    sf::Clock frameClock;

    // When ROCKETSIM_SCREENSHOT is set the finished plot is saved and the
    // window closes at once; used for automated checks.
    const char* screenshot = std::getenv("ROCKETSIM_SCREENSHOT");
    if (screenshot != nullptr)
    {
        replayTime = total;
        if (const char* at = std::getenv("ROCKETSIM_SCREENSHOT_TIME"))   // mid-replay, for checks
        {
            replayTime = std::min(total, std::atof(at));
        }
    }

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>())
            {
                using K = sf::Keyboard::Key;
                if (key->code == K::Escape)
                {
                    window.close();
                }
                else if (key->code == K::Space)
                {
                    replayTime = 0.0;
                }
                else if (key->code == K::F)
                {
                    fit = !fit;
                }
                else if (key->code == K::Equal || key->code == K::Add)
                {
                    speedIndex = std::min(SPEED_COUNT - 1, speedIndex + 1);
                }
                else if (key->code == K::Hyphen || key->code == K::Subtract)
                {
                    speedIndex = std::max(0, speedIndex - 1);
                }
                else if (key->code == K::S)
                {
                    replayTime = total;
                    saveRequested = true;
                }
            }
        }

        const double dt = frameClock.restart().asSeconds();
        if (replayTime < total && screenshot == nullptr)
        {
            replayTime = std::min(total, replayTime + dt * SPEEDS[speedIndex]);
        }

        const Bounds b = frame(plotW / plotH, fit);
        const double spanX = b.maxX - b.minX;
        const double spanY = b.maxY - b.minY;
        auto toPx = [&](double x) { return left + static_cast<float>((x - b.minX) / spanX) * plotW; };
        auto toPy = [&](double y) { return top + static_cast<float>((b.maxY - y) / spanY) * plotH; };
        auto inside = [&](double x, double y) { return x >= b.minX && x <= b.maxX && y >= b.minY && y <= b.maxY; };

        window.clear(BACKGROUND);

        // Grid, ticks, frame, ground.
        sf::VertexArray grid(sf::PrimitiveType::Lines);
        for (double x = std::ceil(b.minX / b.gridX) * b.gridX; x <= b.maxX + 1e-9; x += b.gridX)
        {
            const bool major = std::fabs(std::fmod(x / b.gridX, 5.0)) < 1e-6;
            grid.append(sf::Vertex{{toPx(x), top}, major ? GRID_MAJOR : GRID_MINOR});
            grid.append(sf::Vertex{{toPx(x), top + plotH}, major ? GRID_MAJOR : GRID_MINOR});
            text.draw(window, fmt(x), toPx(x), top + plotH + 14.f, 11, 1, PHOSPHOR_DIM);
        }
        for (double y = std::ceil(b.minY / b.gridY) * b.gridY; y <= b.maxY + 1e-9; y += b.gridY)
        {
            const bool major = std::fabs(std::fmod(y / b.gridY, 5.0)) < 1e-6;
            grid.append(sf::Vertex{{left, toPy(y)}, major ? GRID_MAJOR : GRID_MINOR});
            grid.append(sf::Vertex{{left + plotW, toPy(y)}, major ? GRID_MAJOR : GRID_MINOR});
            text.draw(window, fmt(y), left - 8.f, toPy(y), 11, 2, PHOSPHOR_DIM);
        }
        window.draw(grid);
        if (b.minY <= 0.0 && b.maxY >= 0.0)
        {
            sf::VertexArray ground(sf::PrimitiveType::Triangles);
            thickSegment(ground, {left, toPy(0.0)}, {left + plotW, toPy(0.0)}, 1.5f, PHOSPHOR);
            window.draw(ground);
        }
        sf::RectangleShape frameRect({plotW, plotH});
        frameRect.setPosition({left, top});
        frameRect.setFillColor(sf::Color::Transparent);
        frameRect.setOutlineColor(PHOSPHOR_DIM);
        frameRect.setOutlineThickness(1.f);
        window.draw(frameRect);

        text.draw(window, title_, width / 2.f, 26.f, 18, 1);
        text.draw(window, xLabel_, left + plotW / 2.f, height - 20.f, 13, 1);
        text.draw(window, yLabel_, 22.f, top + plotH / 2.f, 13, 1, PHOSPHOR, -90.f);
        if (flightFrame_)
        {
            text.draw(window, std::string("SCALE ") + (fit ? "FIT" : "1:1") + "   GRID " + fmt(b.gridX / 1000.0) + " km",
                      left, 46.f, 11, 0, PHOSPHOR_DIM);
        }

        // Series up to the replay time, with a glow pass and the line itself.
        // The frame clips what the window shows; points outside are skipped.
        float readoutY = top + 14.f;
        for (std::size_t i = 0; i < series_.size(); ++i)
        {
            const Series& s = series_[i];
            const int style = static_cast<int>(i % STYLE_COUNT);
            std::vector<sf::Vector2f> pts;
            const std::size_t keepEvery = std::max<std::size_t>(1, s.points.size() / 2000);
            const Point* tip = nullptr;
            const Point* previous = nullptr;
            for (std::size_t k = 0; k < s.points.size(); ++k)
            {
                const Point& p = s.points[k];
                if (p.time > replayTime)
                {
                    break;
                }
                previous = tip;
                tip = &p;
                if (k % keepEvery == 0 && inside(p.x, p.y))
                {
                    pts.emplace_back(toPx(p.x), toPy(p.y));
                }
            }
            if (tip != nullptr && inside(tip->x, tip->y))
            {
                pts.emplace_back(toPx(tip->x), toPy(tip->y));
            }
            sf::VertexArray glow(sf::PrimitiveType::Triangles);
            sf::VertexArray line(sf::PrimitiveType::Triangles);
            sf::Color soft = SERIES_COLOUR[style];
            soft.a = 60;
            dashedPolyline(glow, pts, {}, 6.f, soft);
            dashedPolyline(line, pts, SERIES_DASH[style], 1.8f, SERIES_COLOUR[style]);
            window.draw(glow);
            window.draw(line);

            // The rocket: a bright dot at the tip while the replay is running.
            if (tip != nullptr && replayTime < total && inside(tip->x, tip->y))
            {
                sf::CircleShape dot(3.5f);
                dot.setOrigin({3.5f, 3.5f});
                dot.setFillColor(sf::Color::White);
                dot.setPosition({toPx(tip->x), toPy(tip->y)});
                window.draw(dot);
            }

            // Instrument readout for this series.
            sf::VertexArray key(sf::PrimitiveType::Triangles);
            dashedPolyline(key, {{left + plotW - 450.f, readoutY - 1.f}, {left + plotW - 426.f, readoutY - 1.f}},
                           SERIES_DASH[style], 2.f, SERIES_COLOUR[style]);
            window.draw(key);
            std::string readout = s.name;
            if (tip != nullptr && flightFrame_)
            {
                double speed = 0.0;
                if (previous != nullptr && tip->time > previous->time)
                {
                    const double dx = tip->x - previous->x;
                    const double dy = tip->y - previous->y;
                    speed = std::sqrt(dx * dx + dy * dy) / (tip->time - previous->time);
                }
                readout = s.name + "  " + clock(tip->time) + "  ALT " + fixed(tip->y, 0) + "  RNG " + fixed(tip->x, 0) +
                          "  V " + fixed(speed, 0);
            }
            text.draw(window, readout, left + plotW - 418.f, readoutY, 11, 0, SERIES_COLOUR[style]);
            readoutY += 16.f;
        }

        // Markers appear once the replay has reached them.
        for (const Marker& m : markers_)
        {
            if (!inside(m.x, m.y))
            {
                continue;
            }
            bool reached = replayTime >= total;
            for (const Series& s : series_)   // a marker is reached when some series has passed it
            {
                for (const Point& p : s.points)
                {
                    if (p.time <= replayTime && std::fabs(p.x - m.x) < 1e-6 && std::fabs(p.y - m.y) < 1e-6)
                    {
                        reached = true;
                    }
                }
            }
            if (!reached)
            {
                continue;
            }
            sf::CircleShape ring(4.f);
            ring.setOrigin({4.f, 4.f});
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(PHOSPHOR);
            ring.setOutlineThickness(1.5f);
            ring.setPosition({toPx(m.x), toPy(m.y)});
            window.draw(ring);
            if (!m.label.empty())
            {
                text.draw(window, m.label, toPx(m.x) + 8.f, toPy(m.y) - 10.f, 11, 0);
            }
        }

        // Status line.
        const std::string status = clock(replayTime) + " / " + clock(total) + "   SPEED " +
                                   fixed(SPEEDS[speedIndex], 0) + "x   [Space] replay  [+/-] speed  [F] " +
                                   (fit ? "1:1" : "fit") + "  [S] save  [Esc] close";
        text.draw(window, status, left + plotW, 46.f, 11, 2, PHOSPHOR_DIM);

        window.draw(scanlines);
        window.display();

        if (saveRequested)
        {
            saveRequested = false;
            std::cout << (saveScreenshot(window, "plot.png") ? "Saved plot.png\n" : "Could not save plot.png\n");
        }
        if (screenshot != nullptr)
        {
            saveScreenshot(window, screenshot);
            window.close();
        }
    }
}

#endif

// plot.cpp -- GIVEN. See plot.h for the interface. SVG output and ASCII preview;
// the window is in plot_window.cpp.

#include "plot.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace
{
    // The whole display is phosphor green on black, so series are told apart
    // by dash pattern and brightness rather than by hue.
    const char* const SERIES_COLOUR[] = {"#39ff14", "#9dff80", "#39ff14", "#9dff80",
                                         "#39ff14", "#9dff80", "#39ff14", "#9dff80"};
    const char* const SERIES_DASH[] = {"none", "none", "12 6", "12 6", "3 5", "3 5", "14 4 3 4", "14 4 3 4"};
    const int STYLE_COUNT = 8;
    const char* const PHOSPHOR = "#39ff14";
    const char* const PHOSPHOR_DIM = "#1f8a0e";
    const char* const GRID_MINOR = "#0b3a06";
    const char* const GRID_MAJOR = "#146b0a";
    const char* const BACKGROUND = "#000000";

    const char ASCII_GLYPHS[] = {'*', '+', 'o', 'x', '#', '@', '%', '&'};

    // Pick a "nice" tick spacing (1, 2 or 5 times a power of ten).
    double niceStep(double range, int targetTicks)
    {
        if (range <= 0.0)
        {
            return 1.0;
        }
        const double rough = range / targetTicks;
        const double magnitude = std::pow(10.0, std::floor(std::log10(rough)));
        const double residual = rough / magnitude;
        double nice = 10.0;
        if (residual < 1.5)
        {
            nice = 1.0;
        }
        else if (residual < 3.0)
        {
            nice = 2.0;
        }
        else if (residual < 7.0)
        {
            nice = 5.0;
        }
        return nice * magnitude;
    }

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

    std::string fixed1(double value)
    {
        std::ostringstream out;
        out << std::fixed << std::setprecision(1) << value;
        return out.str();
    }

    std::string escapeXml(const std::string& text)
    {
        std::string out;
        for (char c : text)
        {
            switch (c)
            {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                default: out += c; break;
            }
        }
        return out;
    }

    // Layout shared by the SVG and the window so both show the same frame.
    const double WIDTH = 960.0;
    const double HEIGHT = 600.0;
    const double LEFT = 85.0;
    const double RIGHT = 30.0;
    const double TOP = 60.0;
    const double BOTTOM = 70.0;
}

Plot::Plot(const std::string& title, const std::string& xLabel, const std::string& yLabel)
    : title_(title),
      xLabel_(xLabel),
      yLabel_(yLabel),
      flightFrame_(xLabel == "Downrange (m)" && yLabel == "Altitude (m)")
{
}

void Plot::startSeries(const std::string& name)
{
    Series s;
    s.name = name;
    series_.push_back(s);
}

void Plot::addPoint(double x, double y, double time)
{
    if (series_.empty())
    {
        startSeries("flight");
    }
    std::vector<Point>& points = series_.back().points;
    if (time < 0.0)
    {
        time = static_cast<double>(points.size()) * 0.05;   // steady replay, 20 points per second
    }
    points.push_back(Point{x, y, time});
}

void Plot::addMarker(double x, double y, const std::string& label)
{
    Marker m;
    m.x = x;
    m.y = y;
    m.label = label;
    markers_.push_back(m);
}

std::size_t Plot::pointCount() const
{
    std::size_t n = 0;
    for (const Series& s : series_)
    {
        n += s.points.size();
    }
    return n;
}

double Plot::duration() const
{
    double longest = 0.0;
    for (const Series& s : series_)
    {
        if (!s.points.empty())
        {
            longest = std::max(longest, s.points.back().time);
        }
    }
    return longest;
}

Plot::Bounds Plot::fitBounds() const
{
    Bounds b{0.0, 0.0, 0.0, 0.0, 1.0, 1.0};
    for (const Series& s : series_)
    {
        for (const Point& p : s.points)
        {
            if (std::isfinite(p.x) && std::isfinite(p.y))
            {
                b.minX = std::min(b.minX, p.x);
                b.maxX = std::max(b.maxX, p.x);
                b.minY = std::min(b.minY, p.y);
                b.maxY = std::max(b.maxY, p.y);
            }
        }
    }
    for (const Marker& m : markers_)
    {
        b.minX = std::min(b.minX, m.x);
        b.maxX = std::max(b.maxX, m.x);
        b.maxY = std::max(b.maxY, m.y);
    }
    double spanX = b.maxX - b.minX;
    double spanY = b.maxY - b.minY;
    if (spanX < 1e-9)
    {
        spanX = 100.0;
    }
    if (spanY < 1e-9)
    {
        spanY = 100.0;
    }
    b.minX -= 0.05 * spanX;
    b.maxX += 0.05 * spanX;
    b.maxY += 0.08 * spanY;
    if (b.minY < 0.0)
    {
        b.minY -= 0.05 * spanY;
    }
    b.gridX = niceStep(b.maxX - b.minX, 8);
    b.gridY = niceStep(b.maxY - b.minY, 6);
    return b;
}

Plot::Bounds Plot::fixedBounds(double aspect) const
{
    // Base frame: 10 km high, 1 km grid, a 400 m margin below and to the left
    // so the pad and the ground line are clear of the edge. Width follows
    // from the aspect ratio so one metre is the same size on both axes.
    const double baseTop = 10000.0;
    const double baseMargin = 400.0;
    double k = 1.0;
    for (int attempt = 0; attempt < 12; ++attempt)
    {
        const double top = baseTop * k;
        const double margin = baseMargin * k;
        const double right = -margin + (top + margin) * aspect;
        bool fits = true;
        for (const Series& s : series_)
        {
            for (const Point& p : s.points)
            {
                if (p.x > right || p.y > top || p.x < -margin)
                {
                    fits = false;
                }
            }
        }
        if (fits)
        {
            return Bounds{-margin, right, -margin, top, 1000.0 * k, 1000.0 * k};
        }
        k *= 2.0;
    }
    return fitBounds();
}

Plot::Bounds Plot::frame(double aspect, bool fit) const
{
    return (flightFrame_ && !fit) ? fixedBounds(aspect) : fitBounds();
}

bool Plot::saveSvg(const std::string& path) const
{
    std::ofstream out(path);
    if (!out)
    {
        return false;
    }

    const double plotW = WIDTH - LEFT - RIGHT;
    const double plotH = HEIGHT - TOP - BOTTOM;
    const Bounds b = frame(plotW / plotH, false);
    const double spanX = b.maxX - b.minX;
    const double spanY = b.maxY - b.minY;

    auto toPx = [&](double x) { return LEFT + (x - b.minX) / spanX * plotW; };
    auto toPy = [&](double y) { return TOP + (b.maxY - y) / spanY * plotH; };

    out << std::fixed << std::setprecision(1);
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << WIDTH << "\" height=\"" << HEIGHT
        << "\" viewBox=\"0 0 " << WIDTH << " " << HEIGHT << "\">\n";
    out << "<defs><pattern id=\"scan\" width=\"1\" height=\"3\" patternUnits=\"userSpaceOnUse\">"
           "<rect width=\"1\" height=\"1\" fill=\"#000000\" fill-opacity=\"0.35\"/></pattern></defs>\n";
    out << "<rect width=\"100%\" height=\"100%\" fill=\"" << BACKGROUND << "\"/>\n";
    out << "<style>text{font-family:'DejaVu Sans Mono',Consolas,'Courier New',monospace;fill:" << PHOSPHOR
        << "}</style>\n";

    // Title and axis labels.
    out << "<text x=\"" << WIDTH / 2 << "\" y=\"32\" font-size=\"18\" text-anchor=\"middle\">"
        << escapeXml(title_) << "</text>\n";
    out << "<text x=\"" << LEFT + plotW / 2 << "\" y=\"" << HEIGHT - 18
        << "\" font-size=\"13\" text-anchor=\"middle\">" << escapeXml(xLabel_) << "</text>\n";
    out << "<text x=\"20\" y=\"" << TOP + plotH / 2
        << "\" font-size=\"13\" text-anchor=\"middle\" transform=\"rotate(-90 20 " << TOP + plotH / 2
        << ")\">" << escapeXml(yLabel_) << "</text>\n";
    if (flightFrame_)
    {
        out << "<text x=\"" << LEFT << "\" y=\"48\" font-size=\"11\" fill=\"" << PHOSPHOR_DIM << "\">SCALE 1:1   GRID "
            << fmt(b.gridX / 1000.0) << " km</text>\n";
    }

    // Grid: minor lines every step, major every fifth.
    out << "<g stroke-width=\"1\">\n";
    int n = 0;
    for (double x = std::ceil(b.minX / b.gridX) * b.gridX; x <= b.maxX + 1e-9; x += b.gridX, ++n)
    {
        const bool major = std::fabs(std::fmod(x / b.gridX, 5.0)) < 1e-6;
        out << "<line x1=\"" << toPx(x) << "\" y1=\"" << TOP << "\" x2=\"" << toPx(x) << "\" y2=\"" << TOP + plotH
            << "\" stroke=\"" << (major ? GRID_MAJOR : GRID_MINOR) << "\"/>\n";
    }
    for (double y = std::ceil(b.minY / b.gridY) * b.gridY; y <= b.maxY + 1e-9; y += b.gridY)
    {
        const bool major = std::fabs(std::fmod(y / b.gridY, 5.0)) < 1e-6;
        out << "<line x1=\"" << LEFT << "\" y1=\"" << toPy(y) << "\" x2=\"" << LEFT + plotW << "\" y2=\"" << toPy(y)
            << "\" stroke=\"" << (major ? GRID_MAJOR : GRID_MINOR) << "\"/>\n";
    }
    out << "</g>\n";
    out << "<g font-size=\"11\">\n";
    for (double x = std::ceil(b.minX / b.gridX) * b.gridX; x <= b.maxX + 1e-9; x += b.gridX)
    {
        out << "<text x=\"" << toPx(x) << "\" y=\"" << TOP + plotH + 16 << "\" text-anchor=\"middle\">" << fmt(x)
            << "</text>\n";
    }
    for (double y = std::ceil(b.minY / b.gridY) * b.gridY; y <= b.maxY + 1e-9; y += b.gridY)
    {
        out << "<text x=\"" << LEFT - 8 << "\" y=\"" << toPy(y) + 4 << "\" text-anchor=\"end\">" << fmt(y)
            << "</text>\n";
    }
    out << "</g>\n";

    // Ground line and frame.
    if (b.minY <= 0.0 && b.maxY >= 0.0)
    {
        out << "<line x1=\"" << LEFT << "\" y1=\"" << toPy(0.0) << "\" x2=\"" << LEFT + plotW << "\" y2=\"" << toPy(0.0)
            << "\" stroke=\"" << PHOSPHOR << "\" stroke-width=\"1.5\"/>\n";
    }
    out << "<rect x=\"" << LEFT << "\" y=\"" << TOP << "\" width=\"" << plotW << "\" height=\"" << plotH
        << "\" fill=\"none\" stroke=\"" << PHOSPHOR_DIM << "\" stroke-width=\"1\"/>\n";

    // Series, clipped to the frame, thinned so the file stays small. Drawn
    // twice: a wide faint pass for the phosphor glow, then the line itself.
    out << "<clipPath id=\"plotarea\"><rect x=\"" << LEFT << "\" y=\"" << TOP << "\" width=\"" << plotW
        << "\" height=\"" << plotH << "\"/></clipPath>\n";
    out << "<g clip-path=\"url(#plotarea)\" fill=\"none\" stroke-linejoin=\"round\" stroke-linecap=\"round\">\n";
    for (std::size_t i = 0; i < series_.size(); ++i)
    {
        const Series& s = series_[i];
        if (s.points.empty())
        {
            continue;
        }
        const std::size_t keepEvery = std::max<std::size_t>(1, s.points.size() / 2000);
        std::ostringstream pts;
        pts << std::fixed << std::setprecision(1);
        for (std::size_t k = 0; k < s.points.size(); k += keepEvery)
        {
            const Point& p = s.points[k];
            if (std::isfinite(p.x) && std::isfinite(p.y))
            {
                pts << toPx(p.x) << "," << toPy(p.y) << " ";
            }
        }
        const Point& last = s.points.back();
        if (std::isfinite(last.x) && std::isfinite(last.y))
        {
            pts << toPx(last.x) << "," << toPy(last.y);
        }
        const int style = static_cast<int>(i % STYLE_COUNT);
        out << "<polyline stroke=\"" << SERIES_COLOUR[style] << "\" stroke-opacity=\"0.25\" stroke-width=\"6\" points=\""
            << pts.str() << "\"/>\n";
        out << "<polyline stroke=\"" << SERIES_COLOUR[style] << "\" stroke-width=\"1.8\" stroke-dasharray=\""
            << SERIES_DASH[style] << "\" points=\"" << pts.str() << "\"/>\n";
    }
    out << "</g>\n";

    // Markers: a small open circle with a label.
    for (const Marker& m : markers_)
    {
        if (m.x < b.minX || m.x > b.maxX || m.y < b.minY || m.y > b.maxY)
        {
            continue;
        }
        out << "<circle cx=\"" << toPx(m.x) << "\" cy=\"" << toPy(m.y) << "\" r=\"4\" fill=\"none\" stroke=\"" << PHOSPHOR
            << "\" stroke-width=\"1.5\"/>\n";
        if (!m.label.empty())
        {
            out << "<text x=\"" << toPx(m.x) + 8 << "\" y=\"" << toPy(m.y) - 6 << "\" font-size=\"11\">"
                << escapeXml(m.label) << "</text>\n";
        }
    }

    // Readout: the final values of each series, like a stopped telemetry display.
    if (!series_.empty())
    {
        const double lx = LEFT + plotW - 300;
        double ly = TOP + 16;
        out << "<g font-size=\"11\">\n";
        for (std::size_t i = 0; i < series_.size(); ++i)
        {
            const int style = static_cast<int>(i % STYLE_COUNT);
            out << "<line x1=\"" << lx << "\" y1=\"" << ly - 4 << "\" x2=\"" << lx + 24 << "\" y2=\"" << ly - 4
                << "\" stroke=\"" << SERIES_COLOUR[style] << "\" stroke-width=\"2\" stroke-dasharray=\""
                << SERIES_DASH[style] << "\"/>\n";
            std::string readout = escapeXml(series_[i].name);
            if (!series_[i].points.empty() && flightFrame_)
            {
                const Point& p = series_[i].points.back();
                readout += "  T+" + fixed1(p.time) + "s  RNG " + fmt(std::round(p.x)) + " m";
            }
            out << "<text x=\"" << lx + 32 << "\" y=\"" << ly << "\">" << readout << "</text>\n";
            ly += 16;
        }
        out << "</g>\n";
    }

    // Scanlines over everything.
    out << "<rect width=\"100%\" height=\"100%\" fill=\"url(#scan)\"/>\n";
    out << "</svg>\n";
    return static_cast<bool>(out);
}

void Plot::printAscii(int width, int height) const
{
    if (width < 10 || height < 4)
    {
        return;
    }
    const Bounds b = frame(static_cast<double>(width) / height * 0.5, false);   // characters are ~2:1
    const double spanX = b.maxX - b.minX;
    const double spanY = b.maxY - b.minY;

    std::vector<std::string> canvas(height, std::string(width, ' '));
    auto plotChar = [&](double x, double y, char c) {
        if (!std::isfinite(x) || !std::isfinite(y))
        {
            return;
        }
        int col = static_cast<int>((x - b.minX) / spanX * (width - 1));
        int row = static_cast<int>((b.maxY - y) / spanY * (height - 1));
        if (col < 0 || col >= width || row < 0 || row >= height)
        {
            return;
        }
        canvas[row][col] = c;
    };

    if (b.minY <= 0.0 && b.maxY >= 0.0)
    {
        const int groundRow = static_cast<int>((b.maxY - 0.0) / spanY * (height - 1));
        for (int c = 0; c < width; ++c)
        {
            canvas[groundRow][c] = '-';
        }
    }
    for (std::size_t i = 0; i < series_.size(); ++i)
    {
        for (const Point& p : series_[i].points)
        {
            plotChar(p.x, p.y, ASCII_GLYPHS[i % 8]);
        }
    }
    for (const Marker& m : markers_)
    {
        plotChar(m.x, m.y, 'M');
    }

    std::cout << "\n" << title_ << "   (y: " << fmt(b.minY) << " to " << fmt(b.maxY) << " m)\n";
    for (const std::string& row : canvas)
    {
        std::cout << "|" << row << "\n";
    }
    std::cout << "+" << std::string(width, '-') << "\n";
    std::cout << " x: " << fmt(b.minX) << " to " << fmt(b.maxX) << " m";
    for (std::size_t i = 0; i < series_.size(); ++i)
    {
        std::cout << "   " << ASCII_GLYPHS[i % 8] << " = " << series_[i].name;
    }
    std::cout << "\n";
}

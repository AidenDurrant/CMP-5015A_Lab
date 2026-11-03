// plot.h -- draws flight paths in a live window (replayed in real time, like a
//           ground-station display), as an SVG file, and as an ASCII preview.
//
// NOTE -- this file and the plotting was created with the use of AI tools. Declaring for transparency - Aiden
//
// This class is GIVEN and complete. You do not need to change it, but you are
// welcome to read plot.cpp: it uses only std::vector, std::string and
// std::ofstream, all of which you meet in weeks 7 and 9. plot_window.cpp is
// the SFML window and is not lab material.
//
// Usage:
//     Plot plot("My flight");
//     plot.startSeries("Rocket A");
//     plot.addPoint(x, y, t);           // once per time step, with the mission time
//     plot.addMarker(x, y, "apogee");   // optional labelled point
//     plot.saveSvg("flight.svg");
//     plot.show();                      // real-time replay window (needs SFML)
//     plot.printAscii();
//
// Frame of reference. A flight-path plot (the default axis labels) always uses
// the same frame: 1 : 1 scale, 1 km grid, 10 km high and about 16 km wide, with
// the pad at the origin. If a flight overflows it, the frame doubles (2 km grid,
// 20 km high, ...). So the same change in your code always looks the same size
// on the screen, run after run. A plot given other axis labels (range against
// angle, say) auto-scales to its data instead.

#pragma once

#include <string>
#include <vector>

class Plot
{
public:
    explicit Plot(const std::string& title,
                  const std::string& xLabel = "Downrange (m)",
                  const std::string& yLabel = "Altitude (m)");

    // Begin a new line on the plot. Each series gets its own line style and a
    // legend entry. Points are added to the most recently started series.
    void startSeries(const std::string& name);

    // Add one point to the current series. `time` is the mission time in
    // seconds and drives the real-time replay; leave it out for plots that
    // are not flights (the replay then just draws the line steadily).
    void addPoint(double x, double y, double time = -1.0);

    // Add a labelled dot, e.g. apogee or stage separation.
    void addMarker(double x, double y, const std::string& label);

    // Write the plot as an SVG file. Returns false if the file could not be
    // opened.
    bool saveSvg(const std::string& path) const;

    // Open a window and replay the flight in real time. Keys:
    //   Space   restart the replay        + / -   replay speed (1, 2, 5, 10, 20x)
    //   F       fixed frame <-> fit data  S       save plot.png
    //   Esc     close (or close the window)
    // If the program was built without SFML (cmake -DROCKETSIM_LIVE=OFF) this
    // prints a note and returns at once.
    void show() const;

    // Print a coarse character-based version of the plot to std::cout.
    void printAscii(int width = 72, int height = 20) const;

    // Total number of points across all series.
    std::size_t pointCount() const;

private:
    struct Point
    {
        double x;
        double y;
        double time;   // seconds, or a value derived from the index if none was given
    };

    struct Series
    {
        std::string name;
        std::vector<Point> points;
    };

    struct Marker
    {
        double x;
        double y;
        std::string label;
    };

    struct Bounds
    {
        double minX, maxX, minY, maxY;
        double gridX, gridY;   // grid spacing in data units
    };

    // Bounds that fit the data (plus a margin), for auto-scaling plots or the
    // window's "fit" mode.
    Bounds fitBounds() const;

    // The fixed 1 : 1 flight frame for a drawing area of the given aspect
    // ratio (width / height), doubled until the data fits.
    Bounds fixedBounds(double aspect) const;

    // fixedBounds if this is a flight-path plot, else fitBounds.
    Bounds frame(double aspect, bool fit) const;

    // Highest time stamp over all series (the replay's duration).
    double duration() const;

    void drawWindow() const;   // SFML implementation of show(), in plot_window.cpp

    std::string title_;
    std::string xLabel_;
    std::string yLabel_;
    bool flightFrame_;         // true when the default axis labels are in use
    std::vector<Series> series_;
    std::vector<Marker> markers_;
};

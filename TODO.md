// DONE extend index when out of bounds
// DONE get groups x amount of groups from last known data
// DONE create points linked list
// DONE create CSV file reader
// DONE add xwin dimension to group
// DONE draw candlesticks
// DONE return groups as linked list
// DONE keep track of data min and max. we need this info when drawing candlesticks
// DONE segfault when GROUP_SIZE = 1 or 2
// DONE now index data limits are used, but group data limits allows for auto scale
// DONE dataleak for Points, Viewport and groups
// DONE option disable autoscale
//
// TODO find a way to decide on the precision of tickers on axis
// DONE x tickers should follow data not columns
// DONE reindex on second datapoint and calculate spread dynamically
// DONE on reindex first point is added to linked list again
// DONE create update function in index
// DONE rename component to plot oid
// DONE write better makefile
// DONE x should also be double
// DONE plot should have axis and line structs to organize what should be drawn where
// DONE rename pl (plot) to p and p (point) to pnt
// DONE rename pl_ function names to plot_
// DONE don't recreate plot on every iteration, not pretty
// DONE draw legend from axis
// TODO when not using autoscale, it should not change axis scale
// DONE nasty bug in index_get_grouped where we're trying to access a non existing group
// DONE auto resize plot
// DONE status window
// DONE x axis window/struct
// DONE rename axis to yaxis (a -> ya)
// DONE create non candlestick lines
// TODO write a bit of documentation about the way the indexer works before i forget all of it
// DONE test multiple lines on multiple axis
// DONE request many lines from index so we can process them at once in the draw function
// DONE find a better way to sync line id between index and plot
// TODO request groups for specific lines
// TODO draw background colors
// TODO json free all objects
// TODO curses menu system
// TODO fix weird updated candles on new data
// TODO cli switches to select ticker
// TODO Multi row plots
// TODO order book
// DONE volume as second line on left axis
// TODO show/hide lines/axis

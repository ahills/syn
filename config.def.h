
/* Use this if you'd rather run the command with bash than the default shell
 * spawned by system(). An unsafely quoted string containing the command to
 * evaluate is given as the final argument. */
/*#define WRAP_CMD "bash -c"*/

/* Define as a string containing a color specification:
 *   The default terminal color specification is simply "0".
 *   Colors:
 *      black:      "30"
 *      red:        "31"
 *      green:      "32"
 *      yellow:     "33"
 *      blue:       "34"
 *      magenta:    "35"
 *      cyan:       "36"
 *      white:      "37"
 *   To add an effect, prefix with:
 *      none:       "0;" (default)
 *      bold:       "1;"
 *      underscore: "4;"
 *      reverse:    "7"
 *   To change the background color, suffix with a semicolon and then a color
 *   code from the list above.
 *
 *   Example: bold yellow on red background
 *      #define ERROR_COLOR "1;33;31"
 *   Example: red
 *      #define ERROR_COLOR "31" */
#define ERROR_COLOR "31"

/* Verbose information about internals. Probably unwanted. Same color
 * specification as error color. */
/*#define DEBUG*/
/*#define DEBUG_COLOR "1;33"*/

const char *const delim_open = "{{";
const char *const delim_close = "}}";


/*  Copyright 2014 Andrew Hills

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h> /* basename */

#define LEN(X) (sizeof X / sizeof *X)

#include "config.h"

#ifdef ERROR_COLOR
# define E(...) { fflush(stdout); \
                  fprintf(stderr, "%c[%sm", 0x1b, ERROR_COLOR); \
                  fprintf(stderr, __VA_ARGS__); \
                  fprintf(stderr, "%c[0m", 0x1b); \
                  exit(EXIT_FAILURE); }
# define _perror(M) { fflush(stdout); \
                      fprintf(stderr, "%c[%sm", 0x1b, ERROR_COLOR); \
                      perror(M); \
                      fprintf(stderr, "%c[0m", 0x1b); }
#else
# define E(...) { fflush(stdout); \
                  fprintf(stderr, __VA_ARGS__); \
                  exit(EXIT_FAILURE); }
# define _perror(M) { fflush(stdout); perror(M); }
#endif

#ifdef DEBUG
# ifdef DEBUG_COLOR
#  define D(...) { fflush(stdout); \
                   fprintf(stderr, "%c[%sm", 0x1b, DEBUG_COLOR); \
                   fprintf(stderr, __VA_ARGS__); \
                   fprintf(stderr, "%c[0m", 0x1b);}
# else
#  define D(...) { fflush(stdout); fprintf(stderr, __VA_ARGS__); }
# endif
#else
# define D(...)
#endif

static int eval(char *buf);

int
main(int argc, char **argv)
{
    char buf[BUFSIZ + 1] = {0};
    char evalbuf[BUFSIZ + 1] = {0};
    size_t len;
    char *pn, *rest;
    enum {
        SYN_COPY,
        SYN_EVAL,
    } state = SYN_COPY;
    /* Make room for zero-delimiter */
    const size_t maxevallen = LEN(evalbuf) - 1;
    const size_t delim_open_len = strlen(delim_open);
    const size_t delim_close_len = strlen(delim_close);
#ifdef DEBUG
    size_t blockct = 0;
#endif

    pn = argc ? basename(argv[0]) : "syn";

    if(argc > 1)
    {
        fprintf(stderr, "Usage: %s < infile > outfile\n", pn);
#ifdef WRAP_CMD
        fprintf(stderr, "Commands are evaluated using '" WRAP_CMD "'.");
#endif
        exit(EXIT_FAILURE);
    }

    while(!feof(stdin) && (len = fread(buf, sizeof *buf, LEN(buf) - 1, stdin)))
    {
        char *delim;
        D("Read block %zu (%zu bytes)\n", blockct, len);
        rest = buf;
        if(SYN_EVAL == state)
        {
            size_t evallen, newevallen;
            D("  Resuming last block's evaluation (stored %zu bytes)\n",
              strlen(evalbuf));

            /* Parse for delimiter before adding */
            delim = strstr(rest, delim_close);
            evallen = (delim) ? (size_t) (delim - rest) : len;
            newevallen = evallen + strlen(evalbuf);
            D("  Increasing evaluation length by %zu bytes to %zu.\n", evallen,
              newevallen);
            if(newevallen > maxevallen)
            {
                if(delim)
                    *delim = 0;
                E("evaluation too long (%zu > %zu):\n%s%s\n", newevallen,
                  maxevallen, evalbuf, rest);
            }

            if(delim)
            {
                int ret;
                strncat(evalbuf, rest, evallen);
                if((ret = eval(evalbuf)))
                    E("failed to evaluate: (%d)\n%s\n", ret, evalbuf);
                len -= evallen - delim_close_len;
                rest = delim + delim_close_len;
                memset(evalbuf, 0, LEN(evalbuf));
                state = SYN_COPY;
            }
            else
            {
                D("  Saving whole buffer in evaluation buffer (%zu new bytes)."
                  "\n", evallen);
                /* Copy the whole input buffer to the eval buffer */
                strncat(evalbuf, buf, evallen);
                memset(buf, 0, LEN(buf));
                continue;
            }
        }
        D("  Beginning copy with %zu bytes remaining.\n", strlen(rest));
        /* Parse for delimiter before printing */
        while((delim = strstr(rest, delim_open)))
        {
            int ret;
            if(fwrite(rest, sizeof *rest, delim - rest, stdout) !=
               (size_t) (delim - rest))
            {
                _perror("fwrite");
                exit(EXIT_FAILURE);
            }

            D("  Copied %zu bytes.", delim - rest);
            rest = delim + delim_open_len;
            len = strlen(rest);
            D("  %zu bytes remaining to parse.\n", len);
            delim = strstr(rest, delim_close);
            if(!delim)
            {
                /* Store remaining enclosure in temporary buffer */
                D("  Storing the %zu remaining bytes in the evaluation buffer."
                  "\n", len);
                memcpy(evalbuf, rest, len);
                state = SYN_EVAL;
                break;
            }

            /* Evaluate now */
            memset(delim, 0, delim_close_len);
            len = strlen(rest);
            D("  Evaluating a %zu-byte command.\n", len);
            if((ret = eval(rest)))
                E("failed to evaluate: (%d)\n%s\n", ret, rest);
            /* Move on */
            rest = delim + delim_close_len;
            len = strlen(rest);
            D("  %zu bytes remaining to parse.\n", len);
        }
        if(SYN_COPY == state)
        {
            D("  Copying remaining %zu bytes.\n", len);
            if(fwrite(rest, sizeof *rest, len, stdout) != len)
            {
                _perror("fwrite");
                exit(EXIT_FAILURE);
            }
        }

        D("Done with block %zu\n", blockct);
        memset(buf, 0, LEN(buf));
#ifdef DEBUG
        blockct++;
#endif
    }

    switch(state)
    {
        case SYN_COPY:
            exit(EXIT_SUCCESS);
        case SYN_EVAL:
            E("missing closing delimiter:\n%s\n", evalbuf);
    }

    return state; /* unknown state */
}

/* zero-delimited input - returns system() */
static int
eval(char *buf)
{
    int ret;
    char *cmd = buf;
#ifdef WRAP_CMD
    const char *const bash_front = WRAP_CMD " \"";
    const char *const bash_rear  = "\"";
    cmd = calloc(strlen(buf) + strlen(bash_front) + strlen(bash_rear) + 1,
                 sizeof *cmd);
    strcat(cmd, bash_front);
    strcat(cmd, buf);
    strcat(cmd, bash_rear);
    D("constructed command: '%s'\n", cmd);
#endif
    fflush(stdout);
    ret = system(cmd);
    fflush(stdout);
    return ret;
}


/*
 * VIM - Vi IMproved	by Bram Moolenaar
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 */

#ifndef NVIM_VIM_H
# define NVIM_VIM_H

#include "nvim/memory.h"// for xstrlcpy
#include "nvim/types.h"

/* Included when ported to cmake */
/* This is needed to replace TRUE/FALSE macros by true/false from c99 */
#include <stdbool.h>
/* Some defines from the old feature.h */
#define SESSION_FILE "Session.vim"
#define MAX_MSG_HIST_LEN 200
#define SYS_OPTWIN_FILE "$VIMRUNTIME/optwin.vim"
#define RUNTIME_DIRNAME "runtime"
/* end */

/* ============ the header file puzzle (ca. 50-100 pieces) ========= */

#ifdef HAVE_CONFIG_H    /* GNU autoconf (or something else) was here */
# include "auto/config.h"
# define HAVE_PATHDEF

/*
 * Check if configure correctly managed to find sizeof(int).  If this failed,
 * it becomes zero.  This is likely a problem of not being able to run the
 * test program.  Other items from configure may also be wrong then!
 */
# if (SIZEOF_INT == 0)
Error: configure did not run properly.Check auto/config.log.
# endif
#endif

/* user ID of root is usually zero, but not for everybody */
#define ROOT_UID 0


/* Can't use "PACKAGE" here, conflicts with a Perl include file. */
#ifndef VIMPACKAGE
# define VIMPACKAGE     "vim"
#endif

#include "nvim/os_unix_defs.h"       /* bring lots of system header files */

# ifdef HAVE_LOCALE_H
#  include <locale.h>
# endif

/*
 * Maximum length of a path (for non-unix systems) Make it a bit long, to stay
 * on the safe side.  But not too long to put on the stack.
 */
#ifndef MAXPATHL
# ifdef MAXPATHLEN
#  define MAXPATHL  MAXPATHLEN
# else
#  define MAXPATHL  256
# endif
#endif
#ifdef BACKSLASH_IN_FILENAME
# define PATH_ESC_CHARS ((char_u *)" \t\n*?[{`%#'\"|!<")
#else
#  define PATH_ESC_CHARS ((char_u *)" \t\n*?[{`$\\%#'\"|!<")
#  define SHELL_ESC_CHARS ((char_u *)" \t\n*?[{`$\\%#'\"|!<>();&")
#endif

#define NUMBUFLEN 30        /* length of a buffer to store a number in ASCII */

// Make sure long_u is big enough to hold a pointer.
// On Win64, longs are 32 bits and pointers are 64 bits.
// For printf() and scanf(), we need to take care of long_u specifically.
typedef unsigned long long_u;

/*
 * The characters and attributes cached for the screen.
 */
typedef char_u schar_T;
typedef unsigned short sattr_T;
# define MAX_TYPENR 65535

/*
 * The u8char_T can hold one decoded UTF-8 character.
 * We normally use 32 bits now, since some Asian characters don't fit in 16
 * bits.  u8char_T is only used for displaying, it could be 16 bits to save
 * memory.
 */
# ifdef UNICODE16
typedef uint16_t u8char_T;
# else
typedef uint32_t u8char_T;
# endif

#include "nvim/ascii.h"
#include "nvim/keymap.h"
#include "nvim/term_defs.h"
#include "nvim/macros.h"

#include <errno.h>

#include <assert.h>

#include <inttypes.h>
#include <wctype.h>
#include <stdarg.h>

#if defined(HAVE_SYS_SELECT_H) && \
  (!defined(HAVE_SYS_TIME_H) || defined(SYS_SELECT_WITH_SYS_TIME))
# include <sys/select.h>
#endif

/* ================ end of the header file puzzle =============== */

#ifdef HAVE_WORKING_LIBINTL
#  include <libintl.h>
#  define _(x) gettext((char *)(x))
// XXX do we actually need this?
#  ifdef gettext_noop
#    define N_(x) gettext_noop(x)
#  else
#    define N_(x) x
#  endif
#else
#  define _(x) ((char *)(x))
#  define N_(x) x
#  define bindtextdomain(x, y) /* empty */
#  define bind_textdomain_codeset(x, y) /* empty */
#  define textdomain(x) /* empty */
#endif

/* special attribute addition: Put message in history */
#define MSG_HIST                0x1000

/*
 * values for State
 *
 * The lower bits up to 0x20 are used to distinguish normal/visual/op_pending
 * and cmdline/insert+replace mode.  This is used for mapping.  If none of
 * these bits are set, no mapping is done.
 * The upper bits are used to distinguish between other states.
 */
#define NORMAL          0x01    /* Normal mode, command expected */
#define VISUAL          0x02    /* Visual mode - use get_real_state() */
#define OP_PENDING      0x04    /* Normal mode, operator is pending - use
                                   get_real_state() */
#define CMDLINE         0x08    /* Editing command line */
#define INSERT          0x10    /* Insert mode */
#define LANGMAP         0x20    /* Language mapping, can be combined with
                                   INSERT and CMDLINE */

#define REPLACE_FLAG    0x40    /* Replace mode flag */
#define REPLACE         (REPLACE_FLAG + INSERT)
# define VREPLACE_FLAG  0x80    /* Virtual-replace mode flag */
# define VREPLACE       (REPLACE_FLAG + VREPLACE_FLAG + INSERT)
#define LREPLACE        (REPLACE_FLAG + LANGMAP)

#define NORMAL_BUSY     (0x100 + NORMAL) /* Normal mode, busy with a command */
#define HITRETURN       (0x200 + NORMAL) /* waiting for return or command */
#define ASKMORE         0x300   /* Asking if you want --more-- */
#define SETWSIZE        0x400   /* window size has changed */
#define ABBREV          0x500   /* abbreviation instead of mapping */
#define EXTERNCMD       0x600   /* executing an external command */
#define SHOWMATCH       (0x700 + INSERT) /* show matching paren */
#define CONFIRM         0x800   /* ":confirm" prompt */
#define SELECTMODE      0x1000  /* Select mode, only for mappings */

#define MAP_ALL_MODES   (0x3f | SELECTMODE)     /* all mode bits used for
                                                 * mapping */

/* directions */
#define FORWARD                 1
#define BACKWARD                (-1)
#define FORWARD_FILE            3
#define BACKWARD_FILE           (-3)

/* return values for functions */
#if !(defined(OK) && (OK == 1))
/* OK already defined to 1 in MacOS X curses, skip this */
# define OK                     1
#endif
#define FAIL                    0
#define NOTDONE                 2   /* not OK or FAIL but skipped */


/*
 * values for xp_context when doing command line completion
 */
enum {
  EXPAND_UNSUCCESSFUL = -2,
  EXPAND_OK = -1,
  EXPAND_NOTHING = 0,
  EXPAND_COMMANDS,
  EXPAND_FILES,
  EXPAND_DIRECTORIES,
  EXPAND_SETTINGS,
  EXPAND_BOOL_SETTINGS,
  EXPAND_TAGS,
  EXPAND_OLD_SETTING,
  EXPAND_HELP,
  EXPAND_BUFFERS,
  EXPAND_EVENTS,
  EXPAND_MENUS,
  EXPAND_SYNTAX,
  EXPAND_HIGHLIGHT,
  EXPAND_AUGROUP,
  EXPAND_USER_VARS,
  EXPAND_MAPPINGS,
  EXPAND_TAGS_LISTFILES,
  EXPAND_FUNCTIONS,
  EXPAND_USER_FUNC,
  EXPAND_EXPRESSION,
  EXPAND_MENUNAMES,
  EXPAND_USER_COMMANDS,
  EXPAND_USER_CMD_FLAGS,
  EXPAND_USER_NARGS,
  EXPAND_USER_COMPLETE,
  EXPAND_ENV_VARS,
  EXPAND_LANGUAGE,
  EXPAND_COLORS,
  EXPAND_COMPILER,
  EXPAND_USER_DEFINED,
  EXPAND_USER_LIST,
  EXPAND_SHELLCMD,
  EXPAND_CSCOPE,
  EXPAND_SIGN,
  EXPAND_PROFILE,
  EXPAND_BEHAVE,
  EXPAND_FILETYPE,
  EXPAND_FILES_IN_PATH,
  EXPAND_OWNSYNTAX,
  EXPAND_LOCALES,
  EXPAND_HISTORY,
  EXPAND_USER,
  EXPAND_SYNTIME,
};

/* Values for exmode_active (0 is no exmode) */
#define EXMODE_NORMAL           1
#define EXMODE_VIM              2

#ifdef NO_EXPANDPATH
# define gen_expand_wildcards mch_expand_wildcards
#endif

# define HL_CONTAINED   0x01    /* not used on toplevel */
# define HL_TRANSP      0x02    /* has no highlighting	*/
# define HL_ONELINE     0x04    /* match within one line only */
# define HL_HAS_EOL     0x08    /* end pattern that matches with $ */
# define HL_SYNC_HERE   0x10    /* sync point after this item (syncing only) */
# define HL_SYNC_THERE  0x20    /* sync point at current line (syncing only) */
# define HL_MATCH       0x40    /* use match ID instead of item ID */
# define HL_SKIPNL      0x80    /* nextgroup can skip newlines */
# define HL_SKIPWHITE   0x100   /* nextgroup can skip white space */
# define HL_SKIPEMPTY   0x200   /* nextgroup can skip empty lines */
# define HL_KEEPEND     0x400   /* end match always kept */
# define HL_EXCLUDENL   0x800   /* exclude NL from match */
# define HL_DISPLAY     0x1000  /* only used for displaying, not syncing */
# define HL_FOLD        0x2000  /* define fold */
# define HL_EXTEND      0x4000  /* ignore a keepend */
# define HL_MATCHCONT   0x8000  /* match continued from previous line */
# define HL_TRANS_CONT  0x10000 /* transparent item without contains arg */
# define HL_CONCEAL     0x20000 /* can be concealed */
# define HL_CONCEALENDS 0x40000 /* can be concealed */

/* Values for 'options' argument in do_search() and searchit() */
#define SEARCH_REV    0x01  /* go in reverse of previous dir. */
#define SEARCH_ECHO   0x02  /* echo the search command and handle options */
#define SEARCH_MSG    0x0c  /* give messages (yes, it's not 0x04) */
#define SEARCH_NFMSG  0x08  /* give all messages except not found */
#define SEARCH_OPT    0x10  /* interpret optional flags */
#define SEARCH_HIS    0x20  /* put search pattern in history */
#define SEARCH_END    0x40  /* put cursor at end of match */
#define SEARCH_NOOF   0x80  /* don't add offset to position */
#define SEARCH_START 0x100  /* start search without col offset */
#define SEARCH_MARK  0x200  /* set previous context mark */
#define SEARCH_KEEP  0x400  /* keep previous search pattern */
#define SEARCH_PEEK  0x800  /* peek for typed char, cancel search */

/* Values for find_ident_under_cursor() */
#define FIND_IDENT      1       /* find identifier (word) */
#define FIND_STRING     2       /* find any string (WORD) */
#define FIND_EVAL       4       /* include "->", "[]" and "." */

/* Values for file_name_in_line() */
#define FNAME_MESS      1       /* give error message */
#define FNAME_EXP       2       /* expand to path */
#define FNAME_HYP       4       /* check for hypertext link */
#define FNAME_INCL      8       /* apply 'includeexpr' */
#define FNAME_REL       16      /* ".." and "./" are relative to the (current)
                                   file instead of the current directory */

/* Values for buflist_getfile() */
#define GETF_SETMARK    0x01    /* set pcmark before jumping */
#define GETF_ALT        0x02    /* jumping to alternate file (not buf num) */
#define GETF_SWITCH     0x04    /* respect 'switchbuf' settings when jumping */

/* Values for buflist_new() flags */
#define BLN_CURBUF      1       /* May re-use curbuf for new buffer */
#define BLN_LISTED      2       /* Put new buffer in buffer list */
#define BLN_DUMMY       4       /* Allocating dummy buffer */

/* Values for in_cinkeys() */
#define KEY_OPEN_FORW   0x101
#define KEY_OPEN_BACK   0x102
#define KEY_COMPLETE    0x103   /* end of completion */

/* Values for "noremap" argument of ins_typebuf().  Also used for
 * map->m_noremap and menu->noremap[]. */
#define REMAP_YES       0       /* allow remapping */
#define REMAP_NONE      -1      /* no remapping */
#define REMAP_SCRIPT    -2      /* remap script-local mappings only */
#define REMAP_SKIP      -3      /* no remapping for first char */

/* Values returned by mch_nodetype() */
#define NODE_NORMAL     0       /* file or directory, check with os_isdir()*/
#define NODE_WRITABLE   1       /* something we can write to (character
                                   device, fifo, socket, ..) */
#define NODE_OTHER      2       /* non-writable thing (e.g., block device) */

/* Values for readfile() flags */
#define READ_NEW        0x01    /* read a file into a new buffer */
#define READ_FILTER     0x02    /* read filter output */
#define READ_STDIN      0x04    /* read from stdin */
#define READ_BUFFER     0x08    /* read from curbuf (converting stdin) */
#define READ_DUMMY      0x10    /* reading into a dummy buffer */
#define READ_KEEP_UNDO  0x20    /* keep undo info*/

/* Values for change_indent() */
#define INDENT_SET      1       /* set indent */
#define INDENT_INC      2       /* increase indent */
#define INDENT_DEC      3       /* decrease indent */

/* Values for flags argument for findmatchlimit() */
#define FM_BACKWARD     0x01    /* search backwards */
#define FM_FORWARD      0x02    /* search forwards */
#define FM_BLOCKSTOP    0x04    /* stop at start/end of block */
#define FM_SKIPCOMM     0x08    /* skip comments */

/* Values for action argument for do_buffer() */
#define DOBUF_GOTO      0       /* go to specified buffer */
#define DOBUF_SPLIT     1       /* split window and go to specified buffer */
#define DOBUF_UNLOAD    2       /* unload specified buffer(s) */
#define DOBUF_DEL       3       /* delete specified buffer(s) from buflist */
#define DOBUF_WIPE      4       /* delete specified buffer(s) really */

/* Values for start argument for do_buffer() */
#define DOBUF_CURRENT   0       /* "count" buffer from current buffer */
#define DOBUF_FIRST     1       /* "count" buffer from first buffer */
#define DOBUF_LAST      2       /* "count" buffer from last buffer */
#define DOBUF_MOD       3       /* "count" mod. buffer from current buffer */

/* Values for sub_cmd and which_pat argument for search_regcomp() */
/* Also used for which_pat argument for searchit() */
#define RE_SEARCH       0       /* save/use pat in/from search_pattern */
#define RE_SUBST        1       /* save/use pat in/from subst_pattern */
#define RE_BOTH         2       /* save pat in both patterns */
#define RE_LAST         2       /* use last used pattern if "pat" is NULL */

/* Second argument for vim_regcomp(). */
#define RE_MAGIC        1       /* 'magic' option */
#define RE_STRING       2       /* match in string instead of buffer text */
#define RE_STRICT       4       /* don't allow [abc] without ] */

/* values for reg_do_extmatch */
# define REX_SET        1       /* to allow \z\(...\), */
# define REX_USE        2       /* to allow \z\1 et al. */

/* flags for do_ecmd() */
#define ECMD_HIDE       0x01    /* don't free the current buffer */
#define ECMD_SET_HELP   0x02    /* set b_help flag of (new) buffer before
                                   opening file */
#define ECMD_OLDBUF     0x04    /* use existing buffer if it exists */
#define ECMD_FORCEIT    0x08    /* ! used in Ex command */
#define ECMD_ADDBUF     0x10    /* don't edit, just add to buffer list */

/* for lnum argument in do_ecmd() */
#define ECMD_LASTL      (linenr_T)0     /* use last position in loaded file */
#define ECMD_LAST       (linenr_T)-1    /* use last position in all files */
#define ECMD_ONE        (linenr_T)1     /* use first line */

/* flags for do_cmdline() */
#define DOCMD_VERBOSE   0x01    /* included command in error message */
#define DOCMD_NOWAIT    0x02    /* don't call wait_return() and friends */
#define DOCMD_REPEAT    0x04    /* repeat exec. until getline() returns NULL */
#define DOCMD_KEYTYPED  0x08    /* don't reset KeyTyped */
#define DOCMD_EXCRESET  0x10    /* reset exception environment (for debugging)*/
#define DOCMD_KEEPLINE  0x20    /* keep typed line for repeating with "." */

/* flags for beginline() */
#define BL_WHITE        1       /* cursor on first non-white in the line */
#define BL_SOL          2       /* use 'sol' option */
#define BL_FIX          4       /* don't leave cursor on a NUL */

/* flags for mf_sync() */
#define MFS_ALL         1       /* also sync blocks with negative numbers */
#define MFS_STOP        2       /* stop syncing when a character is available */
#define MFS_FLUSH       4       /* flushed file to disk */
#define MFS_ZERO        8       /* only write block 0 */

/* flags for buf_copy_options() */
#define BCO_ENTER       1       /* going to enter the buffer */
#define BCO_ALWAYS      2       /* always copy the options */
#define BCO_NOHELP      4       /* don't touch the help related options */

/* flags for do_put() */
#define PUT_FIXINDENT   1       /* make indent look nice */
#define PUT_CURSEND     2       /* leave cursor after end of new text */
#define PUT_CURSLINE    4       /* leave cursor on last line of new text */
#define PUT_LINE        8       /* put register as lines */
#define PUT_LINE_SPLIT  16      /* split line for linewise register */
#define PUT_LINE_FORWARD 32     /* put linewise register below Visual sel. */

/* flags for set_indent() */
#define SIN_CHANGED     1       /* call changed_bytes() when line changed */
#define SIN_INSERT      2       /* insert indent before existing text */
#define SIN_UNDO        4       /* save line for undo before changing it */

/* flags for insertchar() */
#define INSCHAR_FORMAT  1       /* force formatting */
#define INSCHAR_DO_COM  2       /* format comments */
#define INSCHAR_CTRLV   4       /* char typed just after CTRL-V */
#define INSCHAR_NO_FEX  8       /* don't use 'formatexpr' */
#define INSCHAR_COM_LIST 16     /* format comments with list/2nd line indent */

/* flags for open_line() */
#define OPENLINE_DELSPACES  1   /* delete spaces after cursor */
#define OPENLINE_DO_COM     2   /* format comments */
#define OPENLINE_KEEPTRAIL  4   /* keep trailing spaces */
#define OPENLINE_MARKFIX    8   /* fix mark positions */
#define OPENLINE_COM_LIST  16   /* format comments with list/2nd line indent */

/*
 * There are four history tables:
 */
#define HIST_CMD        0       /* colon commands */
#define HIST_SEARCH     1       /* search commands */
#define HIST_EXPR       2       /* expressions (from entering = register) */
#define HIST_INPUT      3       /* input() lines */
#define HIST_DEBUG      4       /* debug commands */
#define HIST_COUNT      5       /* number of history tables */

/*
 * Flags for chartab[].
 */
#define CT_CELL_MASK    0x07    /* mask: nr of display cells (1, 2 or 4) */
#define CT_PRINT_CHAR   0x10    /* flag: set for printable chars */
#define CT_ID_CHAR      0x20    /* flag: set for ID chars */
#define CT_FNAME_CHAR   0x40    /* flag: set for file name chars */

/*
 * Values for do_tag().
 */
#define DT_TAG          1       /* jump to newer position or same tag again */
#define DT_POP          2       /* jump to older position */
#define DT_NEXT         3       /* jump to next match of same tag */
#define DT_PREV         4       /* jump to previous match of same tag */
#define DT_FIRST        5       /* jump to first match of same tag */
#define DT_LAST         6       /* jump to first match of same tag */
#define DT_SELECT       7       /* jump to selection from list */
#define DT_HELP         8       /* like DT_TAG, but no wildcards */
#define DT_JUMP         9       /* jump to new tag or selection from list */
#define DT_CSCOPE       10      /* cscope find command (like tjump) */
#define DT_LTAG         11      /* tag using location list */
#define DT_FREE         99      /* free cached matches */

/*
 * flags for find_tags().
 */
#define TAG_HELP        1       /* only search for help tags */
#define TAG_NAMES       2       /* only return name of tag */
#define TAG_REGEXP      4       /* use tag pattern as regexp */
#define TAG_NOIC        8       /* don't always ignore case */
# define TAG_CSCOPE     16      /* cscope tag */
#define TAG_VERBOSE     32      /* message verbosity */
#define TAG_INS_COMP    64      /* Currently doing insert completion */
#define TAG_KEEP_LANG   128     /* keep current language */

#define TAG_MANY        300     /* When finding many tags (for completion),
                                   find up to this many tags */

/*
 * Types of dialogs passed to do_vim_dialog().
 */
#define VIM_GENERIC     0
#define VIM_ERROR       1
#define VIM_WARNING     2
#define VIM_INFO        3
#define VIM_QUESTION    4
#define VIM_LAST_TYPE   4       /* sentinel value */

/*
 * Return values for functions like gui_yesnocancel()
 */
#define VIM_YES         2
#define VIM_NO          3
#define VIM_CANCEL      4
#define VIM_ALL         5
#define VIM_DISCARDALL  6

/*
 * arguments for win_split()
 */
#define WSP_ROOM        1       /* require enough room */
#define WSP_VERT        2       /* split vertically */
#define WSP_TOP         4       /* window at top-left of shell */
#define WSP_BOT         8       /* window at bottom-right of shell */
#define WSP_HELP        16      /* creating the help window */
#define WSP_BELOW       32      /* put new window below/right */
#define WSP_ABOVE       64      /* put new window above/left */
#define WSP_NEWLOC      128     /* don't copy location list */


/*
 * flags for check_changed()
 */
#define CCGD_AW         1       /* do autowrite if buffer was changed */
#define CCGD_MULTWIN    2       /* check also when several wins for the buf */
#define CCGD_FORCEIT    4       /* ! used */
#define CCGD_ALLBUF     8       /* may write all buffers */
#define CCGD_EXCMD      16      /* may suggest using ! */

/*
 * "flags" values for option-setting functions.
 * When OPT_GLOBAL and OPT_LOCAL are both missing, set both local and global
 * values, get local value.
 */
#define OPT_FREE        1       /* free old value if it was allocated */
#define OPT_GLOBAL      2       /* use global value */
#define OPT_LOCAL       4       /* use local value */
#define OPT_MODELINE    8       /* option in modeline */
#define OPT_WINONLY     16      /* only set window-local options */
#define OPT_NOWIN       32      /* don't set window-local options */

/* Magic chars used in confirm dialog strings */
#define DLG_BUTTON_SEP  '\n'
#define DLG_HOTKEY_CHAR '&'

/* Values for "starting" */
#define NO_SCREEN       2       /* no screen updating yet */
#define NO_BUFFERS      1       /* not all buffers loaded yet */
/*			0	   not starting anymore */

/* Values for swap_exists_action: what to do when swap file already exists */
#define SEA_NONE        0       /* don't use dialog */
#define SEA_DIALOG      1       /* use dialog when possible */
#define SEA_QUIT        2       /* quit editing the file */
#define SEA_RECOVER     3       /* recover the file */

/*
 * Minimal size for block 0 of a swap file.
 * NOTE: This depends on size of struct block0! It's not done with a sizeof(),
 * because struct block0 is defined in memline.c (Sorry).
 * The maximal block size is arbitrary.
 */
#define MIN_SWAP_PAGE_SIZE 1048
#define MAX_SWAP_PAGE_SIZE 50000

/* Special values for current_SID. */
#define SID_MODELINE    -1      /* when using a modeline */
#define SID_CMDARG      -2      /* for "--cmd" argument */
#define SID_CARG        -3      /* for "-c" argument */
#define SID_ENV         -4      /* for sourcing environment variable */
#define SID_ERROR       -5      /* option was reset because of an error */
#define SID_NONE        -6      /* don't set scriptID */

/*
 * Events for autocommands.
 */
enum auto_event {
  EVENT_BUFADD = 0,             /* after adding a buffer to the buffer list */
  EVENT_BUFNEW,                 /* after creating any buffer */
  EVENT_BUFDELETE,              /* deleting a buffer from the buffer list */
  EVENT_BUFWIPEOUT,             /* just before really deleting a buffer */
  EVENT_BUFENTER,               /* after entering a buffer */
  EVENT_BUFFILEPOST,            /* after renaming a buffer */
  EVENT_BUFFILEPRE,             /* before renaming a buffer */
  EVENT_BUFLEAVE,               /* before leaving a buffer */
  EVENT_BUFNEWFILE,             /* when creating a buffer for a new file */
  EVENT_BUFREADPOST,            /* after reading a buffer */
  EVENT_BUFREADPRE,             /* before reading a buffer */
  EVENT_BUFREADCMD,             /* read buffer using command */
  EVENT_BUFUNLOAD,              /* just before unloading a buffer */
  EVENT_BUFHIDDEN,              /* just after buffer becomes hidden */
  EVENT_BUFWINENTER,            /* after showing a buffer in a window */
  EVENT_BUFWINLEAVE,            /* just after buffer removed from window */
  EVENT_BUFWRITEPOST,           /* after writing a buffer */
  EVENT_BUFWRITEPRE,            /* before writing a buffer */
  EVENT_BUFWRITECMD,            /* write buffer using command */
  EVENT_CMDWINENTER,            /* after entering the cmdline window */
  EVENT_CMDWINLEAVE,            /* before leaving the cmdline window */
  EVENT_COLORSCHEME,            /* after loading a colorscheme */
  EVENT_COMPLETEDONE,           /* after finishing insert complete */
  EVENT_FILEAPPENDPOST,         /* after appending to a file */
  EVENT_FILEAPPENDPRE,          /* before appending to a file */
  EVENT_FILEAPPENDCMD,          /* append to a file using command */
  EVENT_FILECHANGEDSHELL,       /* after shell command that changed file */
  EVENT_FILECHANGEDSHELLPOST,   /* after (not) reloading changed file */
  EVENT_FILECHANGEDRO,          /* before first change to read-only file */
  EVENT_FILEREADPOST,           /* after reading a file */
  EVENT_FILEREADPRE,            /* before reading a file */
  EVENT_FILEREADCMD,            /* read from a file using command */
  EVENT_FILETYPE,               /* new file type detected (user defined) */
  EVENT_FILEWRITEPOST,          /* after writing a file */
  EVENT_FILEWRITEPRE,           /* before writing a file */
  EVENT_FILEWRITECMD,           /* write to a file using command */
  EVENT_FILTERREADPOST,         /* after reading from a filter */
  EVENT_FILTERREADPRE,          /* before reading from a filter */
  EVENT_FILTERWRITEPOST,        /* after writing to a filter */
  EVENT_FILTERWRITEPRE,         /* before writing to a filter */
  EVENT_FOCUSGAINED,            /* got the focus */
  EVENT_FOCUSLOST,              /* lost the focus to another app */
  EVENT_GUIENTER,               /* after starting the GUI */
  EVENT_GUIFAILED,              /* after starting the GUI failed */
  EVENT_INSERTCHANGE,           /* when changing Insert/Replace mode */
  EVENT_INSERTENTER,            /* when entering Insert mode */
  EVENT_INSERTLEAVE,            /* when leaving Insert mode */
  EVENT_JOBACTIVITY,            /* when job sent some data */
  EVENT_MENUPOPUP,              /* just before popup menu is displayed */
  EVENT_QUICKFIXCMDPOST,        /* after :make, :grep etc. */
  EVENT_QUICKFIXCMDPRE,         /* before :make, :grep etc. */
  EVENT_QUITPRE,                /* before :quit */
  EVENT_SESSIONLOADPOST,        /* after loading a session file */
  EVENT_STDINREADPOST,          /* after reading from stdin */
  EVENT_STDINREADPRE,           /* before reading from stdin */
  EVENT_SYNTAX,                 /* syntax selected */
  EVENT_TERMCHANGED,            /* after changing 'term' */
  EVENT_TERMRESPONSE,           /* after setting "v:termresponse" */
  EVENT_USER,                   /* user defined autocommand */
  EVENT_VIMENTER,               /* after starting Vim */
  EVENT_VIMLEAVE,               /* before exiting Vim */
  EVENT_VIMLEAVEPRE,            /* before exiting Vim and writing .viminfo */
  EVENT_VIMRESIZED,             /* after Vim window was resized */
  EVENT_WINENTER,               /* after entering a window */
  EVENT_WINLEAVE,               /* before leaving a window */
  EVENT_ENCODINGCHANGED,        /* after changing the 'encoding' option */
  EVENT_INSERTCHARPRE,          /* before inserting a char */
  EVENT_CURSORHOLD,             /* cursor in same position for a while */
  EVENT_CURSORHOLDI,            /* idem, in Insert mode */
  EVENT_FUNCUNDEFINED,          /* if calling a function which doesn't exist */
  EVENT_REMOTEREPLY,            /* upon string reception from a remote vim */
  EVENT_SWAPEXISTS,             /* found existing swap file */
  EVENT_SOURCEPRE,              /* before sourcing a Vim script */
  EVENT_SOURCECMD,              /* sourcing a Vim script using command */
  EVENT_SPELLFILEMISSING,       /* spell file missing */
  EVENT_CURSORMOVED,            /* cursor was moved */
  EVENT_CURSORMOVEDI,           /* cursor was moved in Insert mode */
  EVENT_TABLEAVE,               /* before leaving a tab page */
  EVENT_TABENTER,               /* after entering a tab page */
  EVENT_SHELLCMDPOST,           /* after ":!cmd" */
  EVENT_SHELLFILTERPOST,        /* after ":1,2!cmd", ":w !cmd", ":r !cmd". */
  EVENT_TEXTCHANGED,            /* text was modified */
  EVENT_TEXTCHANGEDI,           /* text was modified in Insert mode*/
  NUM_EVENTS                    /* MUST be the last one */
};

typedef enum auto_event event_T;

/*
 * Values for index in highlight_attr[].
 * When making changes, also update HL_FLAGS below!  And update the default
 * value of 'highlight' in option.c.
 */
typedef enum {
  HLF_8 = 0         /* Meta & special keys listed with ":map", text that is
                       displayed different from what it is */
  , HLF_AT          /* @ and ~ characters at end of screen, characters that
                       don't really exist in the text */
  , HLF_D           /* directories in CTRL-D listing */
  , HLF_E           /* error messages */
  , HLF_I           /* incremental search */
  , HLF_L           /* last search string */
  , HLF_M           /* "--More--" message */
  , HLF_CM          /* Mode (e.g., "-- INSERT --") */
  , HLF_N           /* line number for ":number" and ":#" commands */
  , HLF_CLN         /* current line number */
  , HLF_R           /* return to continue message and yes/no questions */
  , HLF_S           /* status lines */
  , HLF_SNC         /* status lines of not-current windows */
  , HLF_C           /* column to separate vertically split windows */
  , HLF_T           /* Titles for output from ":set all", ":autocmd" etc. */
  , HLF_V           /* Visual mode */
  , HLF_VNC         /* Visual mode, autoselecting and not clipboard owner */
  , HLF_W           /* warning messages */
  , HLF_WM          /* Wildmenu highlight */
  , HLF_FL          /* Folded line */
  , HLF_FC          /* Fold column */
  , HLF_ADD         /* Added diff line */
  , HLF_CHD         /* Changed diff line */
  , HLF_DED         /* Deleted diff line */
  , HLF_TXD         /* Text Changed in diff line */
  , HLF_CONCEAL     /* Concealed text */
  , HLF_SC          /* Sign column */
  , HLF_SPB         /* SpellBad */
  , HLF_SPC         /* SpellCap */
  , HLF_SPR         /* SpellRare */
  , HLF_SPL         /* SpellLocal */
  , HLF_PNI         /* popup menu normal item */
  , HLF_PSI         /* popup menu selected item */
  , HLF_PSB         /* popup menu scrollbar */
  , HLF_PST         /* popup menu scrollbar thumb */
  , HLF_TP          /* tabpage line */
  , HLF_TPS         /* tabpage line selected */
  , HLF_TPF         /* tabpage line filler */
  , HLF_CUC         /* 'cursurcolumn' */
  , HLF_CUL         /* 'cursurline' */
  , HLF_MC          /* 'colorcolumn' */
  , HLF_COUNT       /* MUST be the last one */
} hlf_T;

/* The HL_FLAGS must be in the same order as the HLF_ enums!
 * When changing this also adjust the default for 'highlight'. */
#define HL_FLAGS {'8', '@', 'd', 'e', 'i', 'l', 'm', 'M', 'n', 'N', 'r', 's', \
                  'S', 'c', 't', 'v', 'V', 'w', 'W', 'f', 'F', 'A', 'C', 'D', \
                  'T', '-', '>', 'B', 'P', 'R', 'L', '+', '=', 'x', 'X', '*', \
                  '#', '_', '!', '.', 'o'}

/*
 * Boolean constants
 */
#ifndef TRUE
# define FALSE  0           /* note: this is an int, not a long! */
# define TRUE   1
#endif

#define MAYBE   2           /* sometimes used for a variant on TRUE */

/*
 * Operator IDs; The order must correspond to opchars[] in ops.c!
 */
#define OP_NOP          0       /* no pending operation */
#define OP_DELETE       1       /* "d"  delete operator */
#define OP_YANK         2       /* "y"  yank operator */
#define OP_CHANGE       3       /* "c"  change operator */
#define OP_LSHIFT       4       /* "<"  left shift operator */
#define OP_RSHIFT       5       /* ">"  right shift operator */
#define OP_FILTER       6       /* "!"  filter operator */
#define OP_TILDE        7       /* "g~" switch case operator */
#define OP_INDENT       8       /* "="  indent operator */
#define OP_FORMAT       9       /* "gq" format operator */
#define OP_COLON        10      /* ":"  colon operator */
#define OP_UPPER        11      /* "gU" make upper case operator */
#define OP_LOWER        12      /* "gu" make lower case operator */
#define OP_JOIN         13      /* "J"  join operator, only for Visual mode */
#define OP_JOIN_NS      14      /* "gJ"  join operator, only for Visual mode */
#define OP_ROT13        15      /* "g?" rot-13 encoding */
#define OP_REPLACE      16      /* "r"  replace chars, only for Visual mode */
#define OP_INSERT       17      /* "I"  Insert column, only for Visual mode */
#define OP_APPEND       18      /* "A"  Append column, only for Visual mode */
#define OP_FOLD         19      /* "zf" define a fold */
#define OP_FOLDOPEN     20      /* "zo" open folds */
#define OP_FOLDOPENREC  21      /* "zO" open folds recursively */
#define OP_FOLDCLOSE    22      /* "zc" close folds */
#define OP_FOLDCLOSEREC 23      /* "zC" close folds recursively */
#define OP_FOLDDEL      24      /* "zd" delete folds */
#define OP_FOLDDELREC   25      /* "zD" delete folds recursively */
#define OP_FORMAT2      26      /* "gw" format operator, keeps cursor pos */
#define OP_FUNCTION     27      /* "g@" call 'operatorfunc' */

/*
 * Motion types, used for operators and for yank/delete registers.
 */
#define MCHAR   0               /* character-wise movement/register */
#define MLINE   1               /* line-wise movement/register */
#define MBLOCK  2               /* block-wise register */

#define MAUTO   0xff            /* Decide between MLINE/MCHAR */

/*
 * Minimum screen size
 */
#define MIN_COLUMNS     12      /* minimal columns for screen */
#define MIN_LINES       2       /* minimal lines for screen */
#define STATUS_HEIGHT   1       /* height of a status line under a window */
#define QF_WINHEIGHT    10      /* default height for quickfix window */

/*
 * Buffer sizes
 */
#ifndef CMDBUFFSIZE
# define CMDBUFFSIZE    256     /* size of the command processing buffer */
#endif

#define LSIZE       512         /* max. size of a line in the tags file */

#define IOSIZE     (1024+1)     /* file i/o and sprintf buffer size */

#define DIALOG_MSG_SIZE 1000    /* buffer size for dialog_msg() */

# define MSG_BUF_LEN 480        /* length of buffer for small messages */
# define MSG_BUF_CLEN  (MSG_BUF_LEN / 6)    /* cell length (worst case: utf-8
                                               takes 6 bytes for one cell) */

/* Size of the buffer used for tgetent().  Unfortunately this is largely
 * undocumented, some systems use 1024.  Using a buffer that is too small
 * causes a buffer overrun and a crash.  Use the maximum known value to stay
 * on the safe side. */
#define TBUFSZ 2048             /* buffer size for termcap entry */

/*
 * Maximum length of key sequence to be mapped.
 * Must be able to hold an Amiga resize report.
 */
#define MAXMAPLEN   50

/* Size in bytes of the hash used in the undo file. */
#define UNDO_HASH_SIZE 32

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef BINARY_FILE_IO
# define WRITEBIN   "wb"        /* no CR-LF translation */
# define READBIN    "rb"
# define APPENDBIN  "ab"
#else
# define WRITEBIN   "w"
# define READBIN    "r"
# define APPENDBIN  "a"
#endif

#ifndef O_NOFOLLOW
# define O_NOFOLLOW 0
#endif

/*
 * defines to avoid typecasts from (char_u *) to (char *) and back
 * (vim_strchr() and vim_strrchr() are now in alloc.c)
 */
#define STRLEN(s)           strlen((char *)(s))
#define STRCPY(d, s)        strcpy((char *)(d), (char *)(s))
#define STRNCPY(d, s, n)    strncpy((char *)(d), (char *)(s), (size_t)(n))
#define STRLCPY(d, s, n)    xstrlcpy((char *)(d), (char *)(s), (size_t)(n))
#define STRCMP(d, s)        strcmp((char *)(d), (char *)(s))
#define STRNCMP(d, s, n)    strncmp((char *)(d), (char *)(s), (size_t)(n))
#ifdef HAVE_STRCASECMP
# define STRICMP(d, s)      strcasecmp((char *)(d), (char *)(s))
#else
# ifdef HAVE_STRICMP
#  define STRICMP(d, s)     stricmp((char *)(d), (char *)(s))
# else
#  define STRICMP(d, s)     vim_stricmp((char *)(d), (char *)(s))
# endif
#endif

/* Like strcpy() but allows overlapped source and destination. */
#define STRMOVE(d, s)       memmove((d), (s), STRLEN(s) + 1)

#ifdef HAVE_STRNCASECMP
# define STRNICMP(d, s, n)  strncasecmp((char *)(d), (char *)(s), (size_t)(n))
#else
# ifdef HAVE_STRNICMP
#  define STRNICMP(d, s, n) strnicmp((char *)(d), (char *)(s), (size_t)(n))
# else
#  define STRNICMP(d, s, n) vim_strnicmp((char *)(d), (char *)(s), (size_t)(n))
# endif
#endif

/* We need to call mb_stricmp() even when we aren't dealing with a multi-byte
 * encoding because mb_stricmp() takes care of all ascii and non-ascii
 * encodings, including characters with umlauts in latin1, etc., while
 * STRICMP() only handles the system locale version, which often does not
 * handle non-ascii properly. */

# define MB_STRICMP(d, s)       mb_strnicmp((char_u *)(d), (char_u *)(s), \
    (int)MAXCOL)
# define MB_STRNICMP(d, s, n)   mb_strnicmp((char_u *)(d), (char_u *)(s), \
    (int)(n))

#define STRCAT(d, s)        strcat((char *)(d), (char *)(s))
#define STRNCAT(d, s, n)    strncat((char *)(d), (char *)(s), (size_t)(n))

# define vim_strpbrk(s, cs) (char_u *)strpbrk((char *)(s), (char *)(cs))

#define MSG(s)                      msg((char_u *)(s))
#define MSG_ATTR(s, attr)           msg_attr((char_u *)(s), (attr))
#define EMSG(s)                     emsg((char_u *)(s))
#define EMSG2(s, p)                 emsg2((char_u *)(s), (char_u *)(p))
#define EMSG3(s, p, q)              emsg3((char_u *)(s), (char_u *)(p), \
    (char_u *)(q))
#define EMSGN(s, n)                 emsgn((char_u *)(s), (int64_t)(n))
#define EMSGU(s, n)                 emsgu((char_u *)(s), (uint64_t)(n))
#define OUT_STR(s)                  out_str((char_u *)(s))
#define OUT_STR_NF(s)               out_str_nf((char_u *)(s))
#define MSG_PUTS(s)                 msg_puts((char_u *)(s))
#define MSG_PUTS_ATTR(s, a)         msg_puts_attr((char_u *)(s), (a))
#define MSG_PUTS_TITLE(s)           msg_puts_title((char_u *)(s))
#define MSG_PUTS_LONG(s)            msg_puts_long_attr((char_u *)(s), 0)
#define MSG_PUTS_LONG_ATTR(s, a)    msg_puts_long_attr((char_u *)(s), (a))

/* Prefer using emsg3(), because perror() may send the output to the wrong
 * destination and mess up the screen. */
#define PERROR(msg) \
  (void) emsg3((char_u *) "%s: %s", (char_u *)msg, (char_u *)strerror(errno))

typedef long linenr_T;                  /* line number type */
typedef int colnr_T;                    /* column number type */
typedef unsigned short disptick_T;      /* display tick type */

#define MAXLNUM (0x7fffffffL)           /* maximum (invalid) line number */
#define MAXCOL (0x7fffffffL)          /* maximum column number, 31 bits */

#define SHOWCMD_COLS 10                 /* columns needed by shown command */
#define STL_MAX_ITEM 80                 /* max nr of %<flag> in statusline */

typedef void        *vim_acl_T;         /* dummy to pass an ACL to a function */

/*
 * fnamecmp() is used to compare file names.
 * On some systems case in a file name does not matter, on others it does.
 * (this does not account for maximum name lengths and things like "../dir",
 * thus it is not 100% accurate!)
 */
#define fnamecmp(x, y) vim_fnamecmp((char_u *)(x), (char_u *)(y))
#define fnamencmp(x, y, n) vim_fnamencmp((char_u *)(x), (char_u *)(y), \
    (size_t)(n))

#if defined(UNIX) || defined(FEAT_GUI)
# define USE_INPUT_BUF
#endif

#ifndef EINTR
# define read_eintr(fd, buf, count) vim_read((fd), (buf), (count))
# define write_eintr(fd, buf, count) vim_write((fd), (buf), (count))
#endif

# define vim_read(fd, buf, count)   read((fd), (char *)(buf), (size_t) (count))
# define vim_write(fd, buf, count)  write((fd), (char *)(buf), (size_t) (count))

/*
 * Enums need a typecast to be used as array index (for Ultrix).
 */
#define hl_attr(n)      highlight_attr[(int)(n)]
#define term_str(n)     term_strings[(int)(n)]

/*
 * vim_iswhite() is used for "^" and the like. It differs from isspace()
 * because it doesn't include <CR> and <LF> and the like.
 */
#define vim_iswhite(x)  ((x) == ' ' || (x) == '\t')

/*
 * EXTERN is only defined in main.c.  That's where global variables are
 * actually defined and initialized.
 */
#ifndef EXTERN
# define EXTERN extern
# define INIT(x)
#else
# ifndef INIT
#  define INIT(x) x
#  define DO_INIT
# endif
#endif

# define MAX_MCO        6       /* maximum value for 'maxcombine' */

/* Maximum number of bytes in a multi-byte character.  It can be one 32-bit
 * character of up to 6 bytes, or one 16-bit character of up to three bytes
 * plus six following composing characters of three bytes each. */
# define MB_MAXBYTES    21

typedef struct timeval proftime_T;

/* Values for "do_profiling". */
#define PROF_NONE       0       /* profiling not started */
#define PROF_YES        1       /* profiling busy */
#define PROF_PAUSED     2       /* profiling paused */


/* Codes for mouse button events in lower three bits: */
# define MOUSE_LEFT     0x00
# define MOUSE_MIDDLE   0x01
# define MOUSE_RIGHT    0x02
# define MOUSE_RELEASE  0x03

/* bit masks for modifiers: */
# define MOUSE_SHIFT    0x04
# define MOUSE_ALT      0x08
# define MOUSE_CTRL     0x10

/* mouse buttons that are handled like a key press (GUI only) */
/* Note that the scroll wheel keys are inverted: MOUSE_5 scrolls lines up but
 * the result of this is that the window moves down, similarly MOUSE_6 scrolls
 * columns left but the window moves right. */
# define MOUSE_4        0x100   /* scroll wheel down */
# define MOUSE_5        0x200   /* scroll wheel up */

# define MOUSE_X1       0x300 /* Mouse-button X1 (6th) */
# define MOUSE_X2       0x400 /* Mouse-button X2 */

# define MOUSE_6        0x500   /* scroll wheel left */
# define MOUSE_7        0x600   /* scroll wheel right */

/* 0x20 is reserved by xterm */
# define MOUSE_DRAG_XTERM   0x40

# define MOUSE_DRAG     (0x40 | MOUSE_RELEASE)

/* Lowest button code for using the mouse wheel (xterm only) */
# define MOUSEWHEEL_LOW         0x60

# define MOUSE_CLICK_MASK       0x03

# define NUM_MOUSE_CLICKS(code) \
  (((unsigned)((code) & 0xC0) >> 6) + 1)


/*
 * jump_to_mouse() returns one of first four these values, possibly with
 * some of the other three added.
 */
# define IN_UNKNOWN             0
# define IN_BUFFER              1
# define IN_STATUS_LINE         2       /* on status or command line */
# define IN_SEP_LINE            4       /* on vertical separator line */
# define IN_OTHER_WIN           8       /* in other window but can't go there */
# define CURSOR_MOVED           0x100
# define MOUSE_FOLD_CLOSE       0x200   /* clicked on '-' in fold column */
# define MOUSE_FOLD_OPEN        0x400   /* clicked on '+' in fold column */

/* flags for jump_to_mouse() */
# define MOUSE_FOCUS            0x01    /* need to stay in this window */
# define MOUSE_MAY_VIS          0x02    /* may start Visual mode */
# define MOUSE_DID_MOVE         0x04    /* only act when mouse has moved */
# define MOUSE_SETPOS           0x08    /* only set current mouse position */
# define MOUSE_MAY_STOP_VIS     0x10    /* may stop Visual mode */
# define MOUSE_RELEASED         0x20    /* button was released */

# if defined(UNIX) && defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SYS_TIME_H)
#  define CHECK_DOUBLE_CLICK 1  /* Checking for double clicks ourselves. */
# endif


/* defines for eval_vars() */
#define VALID_PATH              1
#define VALID_HEAD              2

/* Defines for Vim variables.  These must match vimvars[] in eval.c! */
enum {
    VV_COUNT,
    VV_COUNT1,
    VV_PREVCOUNT,
    VV_ERRMSG,
    VV_WARNINGMSG,
    VV_STATUSMSG,
    VV_SHELL_ERROR,
    VV_THIS_SESSION,
    VV_VERSION,
    VV_LNUM,
    VV_TERMRESPONSE,
    VV_FNAME,
    VV_LANG,
    VV_LC_TIME,
    VV_CTYPE,
    VV_CC_FROM,
    VV_CC_TO,
    VV_FNAME_IN,
    VV_FNAME_OUT,
    VV_FNAME_NEW,
    VV_FNAME_DIFF,
    VV_CMDARG,
    VV_FOLDSTART,
    VV_FOLDEND,
    VV_FOLDDASHES,
    VV_FOLDLEVEL,
    VV_PROGNAME,
    VV_SEND_SERVER,
    VV_DYING,
    VV_EXCEPTION,
    VV_THROWPOINT,
    VV_REG,
    VV_CMDBANG,
    VV_INSERTMODE,
    VV_VAL,
    VV_KEY,
    VV_PROFILING,
    VV_FCS_REASON,
    VV_FCS_CHOICE,
    VV_BEVAL_BUFNR,
    VV_BEVAL_WINNR,
    VV_BEVAL_LNUM,
    VV_BEVAL_COL,
    VV_BEVAL_TEXT,
    VV_SCROLLSTART,
    VV_SWAPNAME,
    VV_SWAPCHOICE,
    VV_SWAPCOMMAND,
    VV_CHAR,
    VV_MOUSE_WIN,
    VV_MOUSE_LNUM,
    VV_MOUSE_COL,
    VV_OP,
    VV_SEARCHFORWARD,
    VV_HLSEARCH,
    VV_OLDFILES,
    VV_WINDOWID,
    VV_PROGPATH,
    VV_JOB_DATA,
    VV_LEN, /* number of v: vars */
};

typedef int VimClipboard;       /* This is required for the prototypes. */


#include "nvim/buffer_defs.h"         /* buffer and windows */
#include "nvim/ex_cmds_defs.h"        /* Ex command defines */
#include "nvim/proto.h"          /* function prototypes */

/* This has to go after the include of proto.h, as proto/gui.pro declares
 * functions of these names. The declarations would break if the defines had
 * been seen at that stage.  But it must be before globals.h, where error_ga
 * is declared. */
#if !defined(FEAT_GUI_W32) && !defined(FEAT_GUI_X11) \
  && !defined(FEAT_GUI_GTK) && !defined(FEAT_GUI_MAC)
# define mch_errmsg(str)        fprintf(stderr, "%s", (str))
# define display_errors()       fflush(stderr)
# define mch_msg(str)           printf("%s", (str))
#else
# define USE_MCH_ERRMSG
#endif




#include "nvim/globals.h"        /* global variables and messages */

/*
 * Return byte length of character that starts with byte "b".
 * Returns 1 for a single-byte character.
 * MB_BYTE2LEN_CHECK() can be used to count a special key as one byte.
 * Don't call MB_BYTE2LEN(b) with b < 0 or b > 255!
 */
# define MB_BYTE2LEN(b)         mb_bytelen_tab[b]
# define MB_BYTE2LEN_CHECK(b)   (((b) < 0 || (b) > 255) ? 1 : mb_bytelen_tab[b])

/* properties used in enc_canon_table[] (first three mutually exclusive) */
# define ENC_8BIT       0x01
# define ENC_DBCS       0x02
# define ENC_UNICODE    0x04

# define ENC_ENDIAN_B   0x10        /* Unicode: Big endian */
# define ENC_ENDIAN_L   0x20        /* Unicode: Little endian */

# define ENC_2BYTE      0x40        /* Unicode: UCS-2 */
# define ENC_4BYTE      0x80        /* Unicode: UCS-4 */
# define ENC_2WORD      0x100       /* Unicode: UTF-16 */

# define ENC_LATIN1     0x200       /* Latin1 */
# define ENC_LATIN9     0x400       /* Latin9 */
# define ENC_MACROMAN   0x800       /* Mac Roman (not Macro Man! :-) */

# ifdef USE_ICONV
#  ifndef EILSEQ
#   define EILSEQ 123
#  endif
#  ifdef DYNAMIC_ICONV
/* On Win32 iconv.dll is dynamically loaded. */
#   define ICONV_ERRNO (*iconv_errno())
#   define ICONV_E2BIG  7
#   define ICONV_EINVAL 22
#   define ICONV_EILSEQ 42
#  else
#   define ICONV_ERRNO errno
#   define ICONV_E2BIG  E2BIG
#   define ICONV_EINVAL EINVAL
#   define ICONV_EILSEQ EILSEQ
#  endif
# endif








/* flags for skip_vimgrep_pat() */
#define VGR_GLOBAL      1
#define VGR_NOJUMP      2

/* behavior for bad character, "++bad=" argument */
#define BAD_REPLACE     '?'     /* replace it with '?' (default) */
#define BAD_KEEP        -1      /* leave it */
#define BAD_DROP        -2      /* erase it */

/* last argument for do_source() */
#define DOSO_NONE       0
#define DOSO_VIMRC      1       /* loading vimrc file */
#define DOSO_GVIMRC     2       /* loading gvimrc file */

/* flags for read_viminfo() and children */
#define VIF_WANT_INFO           1       /* load non-mark info */
#define VIF_WANT_MARKS          2       /* load file marks */
#define VIF_FORCEIT             4       /* overwrite info already read */
#define VIF_GET_OLDFILES        8       /* load v:oldfiles */

/* flags for buf_freeall() */
#define BFA_DEL         1       /* buffer is going to be deleted */
#define BFA_WIPE        2       /* buffer is going to be wiped out */
#define BFA_KEEP_UNDO   4       /* do not free undo information */

/* direction for nv_mousescroll() and ins_mousescroll() */
#define MSCR_DOWN       0       /* DOWN must be FALSE */
#define MSCR_UP         1
#define MSCR_LEFT       -1
#define MSCR_RIGHT      -2

#define KEYLEN_PART_KEY -1      /* keylen value for incomplete key-code */
#define KEYLEN_PART_MAP -2      /* keylen value for incomplete mapping */
#define KEYLEN_REMOVED  9999    /* keylen value for removed sequence */


/* Character used as separated in autoload function/variable names. */
#define AUTOLOAD_CHAR '#'

# define SET_NO_HLSEARCH(flag) no_hlsearch = (flag); set_vim_var_nr( \
    VV_HLSEARCH, !no_hlsearch)

#endif /* NVIM_VIM_H */

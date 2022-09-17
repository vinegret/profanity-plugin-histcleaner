/*
 * Copyright © 2022 Viachaslau Khalikin <viachaslau.vinegret@outlook.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#define _GNU_SOURCE

#include <profapi.h>
#include <sqlite3.h>
#include <stdio.h>

#include "config/files.h"

#define PLUGIN_NAME "clcleaner"
#define PLUGIN_VERSION "0.0.1"

static char *_get_db_filename (const char *const fulljid);
static char *_log_database_clear (const char *const fulljid);
static void _log_database_close (sqlite3 *g_chatlog_database);

void
prof_init (const char *const version, const char *const status,
           const char *const account_name, const char *const fulljid)
{
  char *s = NULL;
  asprintf (&s, "Loaded Plugin %s %s", PLUGIN_NAME, PLUGIN_VERSION);
  prof_cons_show (s);
  free (s);
}

void
prof_on_start (void)
{
}

void
prof_on_shutdown (void)
{
}

void
prof_on_disconnect (const char *const account_name, const char *const fulljid)
{
  char *s = NULL;
  char *result = _log_database_clear (account_name);
  asprintf (&s, "%s", result);
  prof_cons_show (s);
}

void
prof_on_unload (void)
{
  char *s = NULL;
  asprintf (&s, "Unloading Plugin %s %s", PLUGIN_NAME, PLUGIN_VERSION);
  prof_cons_show (s);
  free (s);
}

static char *
_get_db_filename (const char *const fulljid)
{
  return files_file_in_account_data_path (DIR_DATABASE, fulljid, "chatlog.db");
}

static char *
_log_database_clear (const char *const fulljid)
{
  sqlite3 *g_chatlog_database;
  int ret = sqlite3_initialize ();

  char *filename = _get_db_filename (fulljid);
  if (!filename)
    {
      char *s = "Cannot get filename";
      return s;
    }

  ret = sqlite3_open_v2 (filename, &g_chatlog_database, SQLITE_OPEN_READWRITE,
                         NULL);
  free (filename);
  if (ret != SQLITE_OK)
    {
      const char *err_msg = sqlite3_errmsg (g_chatlog_database);
      char *s = NULL;
      asprintf (&s, "Error opening SQLite database: %s", err_msg);
      return s;
    }

  char *err_msg;
  char *query = "DELETE FROM ChatLogs";
  if (SQLITE_OK != sqlite3_exec (g_chatlog_database, query, NULL, 0, &err_msg))
    {
      char *s = NULL;
      if (err_msg)
        {
          asprintf (&s, "SQLite error: %s", err_msg);
          sqlite3_free (err_msg);
        }
      else
        {
          asprintf (&s, "Unknown SQLite error");
        }
      _log_database_close (g_chatlog_database);
      return s;
    }
  _log_database_close (g_chatlog_database);

  return "ChatLogs successfully cleared";
}

static void
_log_database_close (sqlite3 *g_chatlog_database)
{
  if (g_chatlog_database)
    {
      sqlite3_close_v2 (g_chatlog_database);
      sqlite3_shutdown ();
      g_chatlog_database = NULL;
    }
}

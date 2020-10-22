/*
 *  This file is part of halcyon-cmark, a Python3 interface to the
 *  Github Flavored Commonmark C library.
 *
 *  Copyright (C) 2020  Brian Stafford
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <cmark-gfm.h>
#include <cmark-gfm-extension_api.h>

/* these don't appear in the cmark-gfm headers */
cmark_syntax_extension *create_table_extension ();
cmark_syntax_extension *create_strikethrough_extension ();
cmark_syntax_extension *create_tagfilter_extension ();
cmark_syntax_extension *create_autolink_extension ();

typedef struct _CMarkObject CMarkObject;
struct _CMarkObject
  {
    PyObject_HEAD

    cmark_node *document;
    int options;
  };

static void
hycmark_dealloc (CMarkObject *self)
{
  cmark_node_free (self->document);

  Py_TYPE(self)->tp_free ((PyObject *) self);
}

static int
hycmark_init (CMarkObject *self, PyObject *args, PyObject *kwds)
{
  const char *text;
  Py_ssize_t len;
  cmark_parser *parser;
  cmark_syntax_extension *syntax_extension;
  static char *keywords[] =
    {
      "text",
      "options",
      NULL
    };
  int options = CMARK_OPT_SMART
		| CMARK_OPT_FOOTNOTES
		| CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES
		| CMARK_OPT_FULL_INFO_STRING;

  if (!PyArg_ParseTupleAndKeywords (args, kwds, "s#|I", keywords,
				    &text, &len, &options))
    return -1;

  self->options = options;

  parser = cmark_parser_new (self->options);

  //FIXME - make extensions searchable, pass as argument!
  syntax_extension = cmark_find_syntax_extension("table");
  cmark_parser_attach_syntax_extension (parser, syntax_extension);

  syntax_extension = cmark_find_syntax_extension("strikethrough");
  cmark_parser_attach_syntax_extension (parser, syntax_extension);

  syntax_extension = cmark_find_syntax_extension("tasklist");
  cmark_parser_attach_syntax_extension (parser, syntax_extension);

  syntax_extension = cmark_find_syntax_extension("autolink");
  cmark_parser_attach_syntax_extension (parser, syntax_extension);

  cmark_parser_feed (parser, text, len);
  self->document = cmark_parser_finish (parser);
  cmark_parser_free (parser);

  return self->document != NULL ? 0 : -1;
}

static PyObject *
hycmark_html (CMarkObject *self, PyObject *Py_UNUSED(ignored))
{
  PyObject *tail;
  char *text;

  text = cmark_render_html (self->document, self->options, NULL);
  tail = PyUnicode_FromString (text);
  free (text);
  return tail;
}

static PyObject *
hycmark_xml (CMarkObject *self, PyObject *Py_UNUSED(ignored))
{
  PyObject *tail;
  char *text;

  text = cmark_render_xml (self->document, self->options);
  tail = PyUnicode_FromString (text);
  free (text);
  return tail;
}

/* Return sequence of 2 tuples
	((url, title), ...)
   for each link in the document */
static PyObject *
hycmark_links (CMarkObject *self, PyObject *Py_UNUSED(ignored))
{
  cmark_event_type ev_type;
  cmark_iter *iter;
  cmark_node *node;
  PyObject *tuple, *item, *url, *title;
  Py_ssize_t len = 0, elements = 20;

  tuple = PyTuple_New (elements);

  iter = cmark_iter_new (self->document);
  while ((ev_type = cmark_iter_next (iter)) != CMARK_EVENT_DONE)
    if (ev_type == CMARK_EVENT_ENTER)
      {
	node = cmark_iter_get_node (iter);
	switch (cmark_node_get_type (node))
	  {
	  case CMARK_NODE_LINK:
	    url = PyUnicode_FromString (cmark_node_get_url (node));
	    title = PyUnicode_FromString (cmark_node_get_title (node));
	    item = PyTuple_Pack (2, url, title);
	    if (len >= elements)
	      {
		elements = len + 10;
		_PyTuple_Resize (&tuple, elements);
	      }
	    PyTuple_SET_ITEM (tuple, len++, item);
	    break;
	  default:
	    break;
	  }
      }
  cmark_iter_free (iter);

  _PyTuple_Resize (&tuple, len);
  return tuple;
}

/* Update links in parsed document.  The argument should be a dict() object
   with the URL to be replaced as the key and replacement URL as value. */
static PyObject *
hycmark_update_links (CMarkObject *self, PyObject *dict)
{
  cmark_event_type ev_type;
  cmark_iter *iter;
  cmark_node *node;
  PyObject *item;
  const char *url;

  if (!PyDict_Check (dict))
    return NULL;

  iter = cmark_iter_new (self->document);
  while ((ev_type = cmark_iter_next (iter)) != CMARK_EVENT_DONE)
    if (ev_type == CMARK_EVENT_EXIT)
      {
	node = cmark_iter_get_node (iter);
	switch (cmark_node_get_type (node))
	  {
	  case CMARK_NODE_LINK:
	    item = PyDict_GetItemString (dict, cmark_node_get_url (node));
	    if (item != NULL && PyUnicode_Check (item))
	      {
		url = PyUnicode_AsUTF8 (item);
		cmark_node_set_url (node, url);
	      }
	    break;
	  default:
	    break;
	  }
      }
  cmark_iter_free (iter);

  Py_INCREF (Py_None);
  return Py_None;
}

static char *
hycmark_concat (char *str, const char *suffix)
{
  size_t str_len, out_len;
  char *out;

  str_len = str != NULL ? strlen (str) : 0;
  out_len = str_len + strlen (suffix) + 1;
  out = realloc (str, out_len);
  if (out == NULL)
    return str;
  strcpy (&out[str_len], suffix);
  return out;
}

static char *
hycmark_content (cmark_node *root)
{
  cmark_iter *iter;
  cmark_node *node;
  cmark_event_type ev_type;
  const char *text;
  char *out = NULL;

  iter = cmark_iter_new (root);
  while ((ev_type = cmark_iter_next (iter)) != CMARK_EVENT_DONE)
    if (ev_type == CMARK_EVENT_ENTER)
      {
	node = cmark_iter_get_node (iter);
	switch (cmark_node_get_type (node))
	  {
	  case CMARK_NODE_SOFTBREAK:
	  case CMARK_NODE_LINEBREAK:
	    out = hycmark_concat (out, " ");
	    break;
	  default:
	    if ((text = cmark_node_get_literal (node)) != NULL)
	      out = hycmark_concat (out, text);
	    break;
	  }
      }
  cmark_iter_free (iter);
  return out;
}

/* Get the document title.  This is assumed to be the plain text
   content of the first level one header in the document, */
static PyObject *
hycmark_title (CMarkObject *self, PyObject *Py_UNUSED(ignored))
{
  PyObject *obj;
  cmark_event_type ev_type;
  cmark_iter *iter;
  cmark_node *node;
  char *text = NULL;
  int level;

  iter = cmark_iter_new (self->document);
  while ((ev_type = cmark_iter_next (iter)) != CMARK_EVENT_DONE)
    if (ev_type == CMARK_EVENT_ENTER)
      {
	node = cmark_iter_get_node (iter);
	if (cmark_node_get_type (node) != CMARK_NODE_HEADING)
	  continue;
	if ((level = cmark_node_get_heading_level (node)) == 1 || level == 2)
	  {
	    text = hycmark_content (node);
	    break;
	  }
      }
  cmark_iter_free (iter);

  if (text == NULL)
    {
      Py_INCREF (Py_None);
      return Py_None;
    }
  obj = PyUnicode_FromString (text);
  free (text);
  return obj;
}

static PyObject *
hycmark_excerpt (CMarkObject *self, PyObject *Py_UNUSED(ignored))
{
  PyObject *obj;
  cmark_event_type ev_type;
  cmark_iter *iter;
  cmark_node *node;
  char *text = NULL;

  iter = cmark_iter_new (self->document);
  while ((ev_type = cmark_iter_next (iter)) != CMARK_EVENT_DONE)
    if (ev_type == CMARK_EVENT_ENTER)
      {
	node = cmark_iter_get_node (iter);
	if (cmark_node_get_type (node) == CMARK_NODE_PARAGRAPH)
	  {
	    text = hycmark_content (node);
	    break;
	  }
      }
  cmark_iter_free (iter);

  if (text == NULL)
    {
      Py_INCREF (Py_None);
      return Py_None;
    }
  obj = PyUnicode_FromString (text);
  free (text);
  return obj;
}

static PyMethodDef hycmark_methods[] =
  {
    {
      "render_html",
      (PyCFunction) hycmark_html,
      METH_NOARGS,
      "Render the parse tree as HTML"
    },
    {
      "render_xml",
      (PyCFunction) hycmark_xml,
      METH_NOARGS,
      "Render the parse tree as Commonmark XML"
    },
    {
      "links",
      (PyCFunction) hycmark_links,
      METH_NOARGS,
      "Return a sequence of 2-tuples with link (url, title)"
    },
    {
      "update_links",
      (PyCFunction) hycmark_update_links,
      METH_O,
      "Update links in parsed document from dictionary; ie newurl = dict[url]"
    },
    {
      "title",
      (PyCFunction) hycmark_title,
      METH_NOARGS,
      "Return the plain text of the document title (first level 1 or 2 heading)."
    },
    {
      "excerpt",
      (PyCFunction) hycmark_excerpt,
      METH_NOARGS,
      "Return the plain text of the first paragraph."
    },
    { NULL }
  };

static PyTypeObject CMarkType =
  {
    PyVarObject_HEAD_INIT (NULL, 0)
    .tp_name = "hycmark.CMark",
    .tp_doc = "Custom objects for GFM Commonmark",
    .tp_basicsize = sizeof (CMarkObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc) hycmark_init,
    .tp_dealloc = (destructor) hycmark_dealloc,
    .tp_methods = hycmark_methods,
  };

static struct PyModuleDef moduledef =
  {
    PyModuleDef_HEAD_INIT,
    .m_name = "hycmark",
    .m_doc = "Utility functions for cmark and cmark-gfm API.",
    .m_size = -1,
  };

void cmark_gfm_core_extensions_ensure_registered (void);

PyMODINIT_FUNC
PyInit_hycmark (void)
{
  PyObject *module;

  if (PyType_Ready (&CMarkType) < 0)
    return NULL;

  if ((module = PyModule_Create (&moduledef)) == NULL)
    return NULL;

  Py_INCREF (&CMarkType);
  if (PyModule_AddObject (module, "CMark", (PyObject *) &CMarkType) < 0)
    {
      Py_DECREF (&CMarkType);
      Py_DECREF (module);
      return NULL;
    }

  PyModule_AddIntConstant (module, "OPT_DEFAULT", CMARK_OPT_DEFAULT);
  PyModule_AddIntConstant (module, "OPT_SOURCEPOS", CMARK_OPT_SOURCEPOS);
  PyModule_AddIntConstant (module, "OPT_HARDBREAKS", CMARK_OPT_HARDBREAKS);
  PyModule_AddIntConstant (module, "OPT_UNSAFE", CMARK_OPT_UNSAFE);
  PyModule_AddIntConstant (module, "OPT_NOBREAKS", CMARK_OPT_NOBREAKS);
  PyModule_AddIntConstant (module, "OPT_NORMALIZE", CMARK_OPT_NORMALIZE);
  PyModule_AddIntConstant (module, "OPT_VALIDATE_UTF8",
			   CMARK_OPT_VALIDATE_UTF8);
  PyModule_AddIntConstant (module, "OPT_SMART", CMARK_OPT_SMART);
  PyModule_AddIntConstant (module, "OPT_GITHUB_PRE_LANG",
			   CMARK_OPT_GITHUB_PRE_LANG);
  PyModule_AddIntConstant (module, "OPT_LIBERAL_HTML_TAG",
			   CMARK_OPT_LIBERAL_HTML_TAG);
  PyModule_AddIntConstant (module, "OPT_FOOTNOTES", CMARK_OPT_FOOTNOTES);
  PyModule_AddIntConstant (module, "OPT_STRIKETHROUGH_DOUBLE_TILDE",
			   CMARK_OPT_STRIKETHROUGH_DOUBLE_TILDE);
  PyModule_AddIntConstant (module, "OPT_TABLE_PREFER_STYLE_ATTRIBUTES",
			   CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES);
  PyModule_AddIntConstant (module, "OPT_FULL_INFO_STRING",
			   CMARK_OPT_FULL_INFO_STRING);

  cmark_gfm_core_extensions_ensure_registered ();

  return module;
}

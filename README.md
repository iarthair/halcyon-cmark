# Halcyon CMark

Halcyon CMark a simple python interface to call the
[cmark-gfm](https://github.com/github/cmark-gfm) API.

Unlike most other Python modules to interface to Commonmark, this one does not
make you use a modified version of cmark-gfm, just install the version that is
packaged by your distribution or build the version from GitHub.

Halcyon because it is simple and frustration free.

## Prerequisites

Install [cmark-gfm](https://github.com/github/cmark-gfm) if you want to build
the `master` branch.

If building the `cmark-node-id` branch, you'll need to install the `node-id`
branch from the local copy of the
[cmark-gfm repository](https://github.com/iarthair/cmark-gfm.git)
instead.

### What is the cmark-node-id branch?

The cmark-node-id branch adds a `toc()` method which locates headings in the
document, assigns an `id` attribute (`xml:id` for XML rendering) to each and
returns a list of 3-tuples for each heading. This allows applications to create
links to reference the headings.  This relies on 2 API calls not in the
standard `cmark-gfm` library, but present in the local fork.

## Install

This is straightforward. Download source, make sure cmark-gfm is installed,
then:

```sh
$ python setup.py build
$ sudo python setup.py install
```

Python3 is assumed.  This might build for Python2.7 but I have not checked this.

## Copyright and Licence

This software is © 2020 Brian Stafford and the licence is
[LGPL 2.1](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html).

## Quick Usage

```python
import hycmark

markdown='''# Hello *World*

Just a quick test!
'''

md = hycmark.CMark(markdown)
print(md.render_html())
```

produces

```html
<h1>Hello <em>World</em></h1>
<p>Just a quick test!</p>
```

## Methods

Constructor/Method | Return | Description
---:|:---:|:---
CMark(text, options=_defaults_) | instance | Read and parse markdown document.
render_html() | html-text | Render parsed document as HTML.
render_xml() | xml-text | Render parsed document as Commonmark XML.
title() | title-text | Plain text of the document title, assumed to be the first level 1 or 2 heading.
excerpt() | paragraph-text | Plain text of the first paragraph.
links() | tuple | Return a sequence of (url, title) tuples for each link in the document.
update_links(dict) | | Update links in document from a dictionary, where the key is the URL to update and value is the replacement.

The options values for the constructor are a bitwise-or of the following
values, these have the same meanings and values as their counterparts in the
cmark-gfm C API.

* hycmark.OPT_DEFAULT
* hycmark.OPT_SOURCEPOS
* hycmark.OPT_HARDBREAKS
* hycmark.OPT_UNSAFE
* hycmark.OPT_NOBREAKS
* hycmark.OPT_NORMALIZE
* hycmark.OPT_VALIDATE_UTF8
* hycmark.OPT_SMART
* hycmark.OPT_GITHUB_PRE_LANG
* hycmark.OPT_LIBERAL_HTML_TAG
* hycmark.OPT_FOOTNOTES
* hycmark.OPT_STRIKETHROUGH_DOUBLE_TILDE
* hycmark.OPT_TABLE_PREFER_STYLE_ATTRIBUTES
* hycmark.OPT_FULL_INFO_STRING


The `title()` and `excerpt()` methods are useful for generating an index of
documents.  The `links()` and `update_links()` methods can be used to fix up
links before rendering the document. For example:

```python
import hycmark
markdown = '...'

md = hycmark.CMark(markdown)
print(md.links())
md.update_links({'example.md': 'example.html'})
print(md.render_html())
```

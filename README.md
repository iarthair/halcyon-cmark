# Halcyon CMark

Halcyon CMark a simple python interface to call the
[cmark-gfm](https://github.com/github/cmark-gfm) API.

Unlike most other Python modules to interface to Commonmark, this one does not
make you use a modified version of cmark-gfm, just install the version that is
packaged by your distribution or build the version from GitHub.

Halcyon because it is simple and frustration free.

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

## Usage

```python
import cmark

markdown='''# Hello *World*

Just a quick test!
'''

md = cmark.CMark(markdown)
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
render_html() | str | Render parsed document as HTML.
render_xml() | str | Render parsed document as Commonmark XML.
links() | tuple | Return a sequence of (url, title) tuples for each link in the document.
update_links(dict) | | Update links in document from a dictionary, where the key is the URL to update and value is the replacement.

The `links()` and `update_links()` methods can be used to fix up links
before rendering the document. For example:

```python
import cmark
markdown = '...'

md = cmark.CMark(markdown)
print(md.links())
md.update_links({'example.md': 'example.html'})
print(md.render_html())
```

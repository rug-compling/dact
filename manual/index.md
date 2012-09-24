---
layout: manual
title: Manual - Decaffeinated Alpino Corpus Tool
---

* Table of Contents
{:toc}

## Introduction

Dact is an application for browsing and searching syntactically
annotated treebanks such as Lassy Small and Lassy Large. This manual
provides an introduction to the Dact interface and its query language.
If you are already familiar with Dact and want to see a more in-depth
treatment of interesting linguistic phenomena, you might be interested
in the [Dact Cookbook](../cookbook).

## Installing and starting Dact

Dact for Windows, Mac OS X, and Ubuntu can be obtained via the [Dact
website](..). Here, we provide instructions for installing and starting
Dact.

### Windows

After downloading the Windows installer, run it. The installer will
copy Dact to your *Program Files* directory and add a *Start Menu* item.

On Windows XP, Vista, and 7, you can start Dact by clicking on the
*Start Menu* and selecting Dact from the list of programs. On Windows
Vista, 7, and 8, you can press the Windows key and type *dact*. Dact
should appear in the list of programs, where you can select it.

### Mac OS X

Dact is provided as a disk image. Click the on the disk image in your
browser or double click on the disk image in Finder to open it. Then
drag Dact to your *Applications* folder.

You can start Dact by double clicking on Dact in Finder. It's usually
faster to start Dact from Spotlight. Press *Cmd + Space* to open the
Spotlight search popup, type *dact*, and press *Return*.

### Ubuntu

Dact is can be installed through Ubuntu's package manager. Open a Terminal
and execute the following commands:

    sudo add-apt-repository ppa:danieldk/dact
    sudo apt-get update
    sudo apt-get install alpino-dact

To start Dact, press the *Super* (Windows) key to bring up the Dash,
type *dact*, and press *Return*. You can also launch Dact in a terminal
using the <tt>alpino-dact</tt> command.

## Getting started

Once Dact is started, the *Open Corpus* dialog will pop up:

![](images/open-corpus-window.png)

You have three options to open a corpus:

* **Opening a publicly available corpus:** The dialog shows a list of
  corpora that are publicly available. Selecting a corpus from the list
  will open it. If the corpus is not available on your computer, Dact
  will download it for you.

* **Opening another corpus:** If you have a corpus that is not publicly
  available, such as Lassy Small, you can open it by clicking on the
  *Open local file* button (or press *Ctrl+l* or *Cmd+l on a Mac) and
  selecting that corpus.

* **Opening a directory with corpora:** You can also open a directory
  with corpora using the *Open directory* button. This is especially
  useful for large corpora, such as Lassy Large, which are distributed
  as a collection of smaller Dact corpora.

After opening a corpus, the window will resemble the following window:

![](images/dact-osx.png)

The main Dact Window consists of a filter field and three tabs:

-   The Tree tab shows all entries with matching nodes in the corpus
    that match your filter query. You can click on an entry to show it
    dependency tree. You can also open the Inspector using the button in
    the upper right corner (*Ctrl+i*) and inspect the attributes of
    individual nodes in the dependency tree by selecting them.

-   The Statistics tab shows the occurrences of values of a specific
    attribute for all the nodes in the corpus that match your filter
    query. You can specify which attribute with the dropdown menu on
    this tab.

-   The Sentences tab shows the entries containing matching nodes as
    sentences, and highlights them.

Although the corpus can be browsed entry by entry, most functionality of
Dact is query-driven. After a short introduction to the query language
in the next section, you will be prepared to use other functionality of
Dact.

## Queries

Queries are written in the [XPath](http://en.wikipedia.org/wiki/XPath)
query language.

### Matching a node

Every node in the tree is represented as an *node* element. You can
match any node in the tree by using two forward slashes:

    //node

Of course, normally, you would want to match nodes with certain
restrictions based on attributes of a node. Such restrictions can be
entered between square brackets (`[` and `]`). And attributes are
prefixed by the ‘at’ sign (`@`). Commonly-used attributes are:

rel
:   relation label

cat
:   category

pos
:   part of speech tag

root
:   the root/stem of a lexical node

For instance, the following query will match all nodes with the *pos*
attribute, or in other words lexical nodes:

    //node[@pos]

We can also restrict the selection by requiring that an attribute has a
specific value using the equals sign (*=*). For instance, the following
query will match all nodes, which have a *pos* attribute with the value
*det*:

    //node[@pos="det"]

Such conditions can also be combined. Using the *and* operator will
require both conditions to be true, while the *or*operator requires one
of the conditions to be true. The following query will match all nodes
with a *su* dependency relation, that also have *det* as their part of
speech tag:

    //node[@rel="su" and @pos="det"]

There are some functions available in XPath which may be useful. For
example using `not` we could find any node that does not match a certain
condition. Say we want to match everything except nouns, we could write:

    //node[not(@pt="n")]

Or say we wanted to match everything except nouns that are lexical nodes
starting with the letter *v*. We can use the `starts-with` function to
require that the *root* attribute starts with the text *v*. The *and*
operator will tie them together.

    //node[not(@pt) and starts-with(@root, "v")]

`contains` is another function that works just like `start-with`, except
it match if the text is found anywhere in the attribute its value, not
just at the beginning.

We can also make queries based on the structure of a tree. For example,
the following query will match any node with a *su* dependency relation
that has a determiner: one of the children of the matching node is a
node which*pos* attribute has the value *det*.

    //node[@rel="su" and node[@pos="det"]]

Now that query matched the *su* node, but we can also match the *det*
node. This is useful in the Statistics Window, where the matching nodes
are read. This query will do just that:

    //node[@rel="su"]/node[@pos="det"]

It first finds the subject nodes, and then matches all the determiners
found in these nodes. We can continue this to for example find all the
nouns in the noun phrase in a preposition phrase. We first find the
preposition phrase somewhere in the tree (mind the double slash), then
find the noun phrase among one of its children (the single slash), and
then find a noun among the noun phrase its children.

    //node[@cat="pp"]/node[@cat="np"]/node[@pt="n"]

This goes down deeper into the tree, but we can also move back up in the
tree using double dots. Say we wanted to select all the siblings of a
noun node, we can use .. to move up to the parent of the noun node, and
then select all the children of this parent node:

    //node[@pt="n"]/../node

Or we could select all nodes of which the parent node has a child node
which is a noun:

    //node[../node[@pt="n"]]

Note that strings, i.e. the text between quotes and attributes can be
used interchangeably since an attribute has a string as a value. For
example, say we would want to do something silly and try to find all
lexical nodes with an attribute *pt* that has the same value al the word
of the node, which would mostly be just the letter *n*. The word is
accessible through the *word* attribute. So we end up comparing the
*word* attribute with the *pt* attribute:

    //node[@pt=@word]

And we are not just bound to the attributes of the current node. Say we
wanted to find examples the dutch verb*krijgen* used in a passive form.
To do this, we have to look for sentences where the subject of the
sentence is also the object of the verb phrase. A translation takes
place. In the corpus this is indicated by an *index* attribute. This
attribute contains the same value on both nodes. I.e.. when we want to
see translation 1, this query will highlight both the nodes before and
after the translation:

    //node[@index=1]

Now using that knowledge we can find the node which contains the verb
*krijgen*, a subject, and a object in the verb complement which both
share the same value for *index*:

    //node[ node[@rel="hd" and @root="krijg"] and node[@rel="su"]/@index=node[@rel="vc"]/node[@rel="obj2"]/@index ]

(This query was taken from the [manual for Treebank
Tools](http://www.let.rug.nl/~vannoord/alp/Alpino/TreebankTools.html),
which contains some more interesting queries.)

> **Note**
>
> When using the position attributes, `@begin` and `@end`, note that
> XPath sees them as strings. If you want to compare them, you may want
> to convert them to numbers first using `number()`. For example:
>
>     //node[@cat="pp" and node[@rel="hd"]/number(@begin) > node[@rel="obj1"]/number(@begin)]

> **Note**
>
> Please do note that Dact expects queries that return nodes. A
> highlight query returning the value of an attribute wont highlight any
> nodes.

### Exploring a corpus

The left pane on the Tree tab shows a list of corpus entries, where each
entry represents a parsed sentence containing at least one node that
matched your filter query.

After typing the query, press the *Enter* key, and Dact will start
filtering the corpus. If you want to interrupt filtering, press the
*Esc* key. You can also pick one of your previous queries form the
history using the arrow on the right of the filter field.

Using the *Next* and *Previous* arrows in the top left menu bar, you can
walk through each found entry. Or you can use the *Ctrl+Down* and
*Ctrl+Up* shortcuts.

### Highlighting nodes

After selecting an entry, its parse tree is shown in the right pane. To
easier to spot interesting phenomena, or test a query, you can enter a
separate query in the highlight field. Each node matching the highlight
query will be colored:

![](images/highlighted-nodes.png)

Initially, the filter query is used as the highlight query.

Matching nodes will be highlighted in the tree in green (you can alter
this color in the Preferences). The buttons *Zoom In* and *Zoom Out*
can be used to scale the tree. *Previous Node* and *Next Node* will walk
you through all the matching nodes. You can use *Ctrl+Left*
and*Ctrl+Right* as well. The focussed node will then be marked by a
slightly thicker border. Normally, the scroll wheel is used for panning
the tree. but when you press *Ctrl*, scrolling will cause the tree to
scale. *Ctrl+=* and *Ctrl+-* can also be used to zoom in and out, and
*Ctrl+0* resets the zoom level to show the whole tree, just like *Fit*
button on the toolbar does.

The leaf nodes have tool tips showing more details about the node.

Below the tree the sentence is shown, and the parts in the sentence
represented by the matching nodes are highlighted.

### Inspecting nodes

If you want to know more about a node, you can select it and open the
Inspector by clicking its button on the right hand side of the toolbar,
pressing *Ctrl+i* or by enabling it in the *View* menu. This is very
useful for writing queries.

![](images/inspector.png)

The Inspector will show you every attribute the focussed node has. You
can right-click an attribute and use the context menu to directly add it
to your search query.

> **Tip**
>
> The Inspector is detachable and can be torn of the Main window.

### Gathering statistics

The Statistics tab shows which values can be found for a certain
attribute of the matching nodes throughout the corpus. These nodes can
be filtered using the same XPath queries. If a node does match the
filter, but does not have the attribute, it will be counted as *[missing
attribute]*.

![](images/statistics-tab.png)

Make sure to use a filter which matches the nodes you want to know the
values of. For example, say we wanted to know how often every
preposition occurs in a preposition phrase. We need to filter for the
preposition nodes that are children of a preposition phrase node:

    //node[@cat="pp"]/node[@pt="vz"]

If you are unsure whether your filter will match too many or too little
nodes, try to test it visually in the Tree tab by using your query as
the highlight query.

Because we want to know how often the word occurs, we select the `word`
or `lemma` attribute from the drop-down menu.

The *Value* column shows all the distinct values found, and the *Nodes*
column shows how often nodes with these values where encountered. The
*percentage* column puts this number into perspective by showing how
much this is as a percentage of the total count of found values. This
total is shown in the bottom right of the window, as is the number of
distinct values (i.e. the number of rows in the table).

You can double-click one of the rows to search for all nodes that
together are summed up in that row. Dact will automatically generate a
new filter query for you.

When copying rows to the clipboards, they will be pasted as a
tab-separated plain-text table. Excel and many other programs are able
to import this format when pasting it into a document.

### Exporting Statistics

The results from the Statistics tab can be saved in various formats:
plain text, HTML, Excel worksheet and CSV. Go to the *File - Save as…*
menu and select the file type from the drop-down menu in the save
dialog.

> **Tip**
>
> Note that this menu item also works in the *Sentences* tab.

### Sentences

To quickly get an impression which part of a sentence matches a query,
you can use the *Sentences* tab.

![](images/sentences-tab.png)

The window highlights the part of a sentence which matches the query for
all the sentences in the corpus where at least one matching node is
found.

You can select alternative display modes using the drop-down menu.
Currently three methods are implemented:

-   *Complete Sentence* shows the matching nodes in the sentence on a
    different background. Nested matches have a more opaque background.
    The color can be changed in Dact's Preference Window.

-   *Only Matches* shows only partial sentences of the nodes that
    matched.

-   *Keywords in Context* shows all the matches directly underneath each
    other and prints the rest of the sentence left and right of the
    match. These colors can also be configured in Dact's Preference
    Window.

## Converting corpora

Dact can only work with Dact corpora, but it can create these from any
Directory and Compact corpus. To convert a Directory or Compact corpus,
go to the *Tools - Convert corpus* submenu, and choose your type. Dact
will prompt you for the location of your corpus, and where to save the
new Dact corpus. Afterwards you can open the newly generated Dact corpus
using the *File - Open…* menu item.

> **Note**
>
> Note that *your original corpus won't be affected* in this process.

## Configuring Dact

Dact has some preferences you can change it to suit more to your needs.
You can configure the font and colors of the interface. You can find
these in the Preference window, which you can find in *Edit* menu. (On
OS X, you will find it in the application menu like any other OS X
application.) Changes are automatically applied and saved.

If you accidentally mess up, you can always return to the default
preferences Dact ships with by clicking the *Return to Default* button
in a tab. The preferences will be restored to their default values.

![](images/preferences-window.png)

### Font and colors

Dact allows you to choose your own colors and font used in the
interface to some extend.

Note that the OS X version of Dact does not have a *Font* tab. In the
other versions of Dact you can change the font that is used for the
buttons and lists throughout Dact.

### Network

Dact's *Download corpus*, *Open remote corpus* and *Parse sentences*
features use web services to function. The addresses of these web
services can be changed.

> **Note**
>
> Note that not all of these features may be enabled in your version of
> Dact as of the time of writing not all of the required web services
> are publicly available.

## Macros

Dact supports macros in its XPath queries. You can insert simple
placeholders in your query which are expanded before a query is
evaluated. A placeholder is the name of the macro surrounded by
percentage signs (`%`).

### Using a macro

Given you have loaded a file with the following macro:

    interesting = """ @rel="su" or @rel="vc" """

When you enter the following query in to the filter query field:

    //node[%interesting%]

Dact replace the placeholder, and execute the following query:

    //node[ @rel="su" or @rel="vc" ]

Since Dact only does simple text replacement, you can also use invalid
or partial XPath code as replacement. As long as the expanded query is
valid, Dact will accept it.

### Loading a file with Macros

You can load a file with macros with the *Macros - Load file…* menu
item. When the file is successfully loaded, a submenu with the same name
as the filename is visible in the *Macros* menu, containing all the
macros in the file.

When you have focused one of the XPath query fields in Dact, you can
select one of the macros from the *Macros* menu to insert it at the
position of your text cursor.

### Macros file syntax

Macro files are rather simple and can be created with any plain text
editor. Macros have the following syntax:

    placeholder = """replacement"""

`placeholder` is the name of your macro, and `replacement` the XPath
code it will be replaced with. The name and replacement are separated by
a `=`, and the replacement has to be surrounded by three pairs of double
quotes. `replacement` may span several lines.

Dact also expands macros while loading them. This way, you can use
macros you defined earlier on in a file in your macros.

    b = """number(@begin)"""
    e = """number(@end)"""

    headrel = """ ( @rel="hd" or @rel="cmp" or @rel="mwp" or @rel="crd" or @rel="rhd" or @rel="whd" or @rel="nucl" or @rel="dp") """

    precedes_head_of_smain = """
    (  ancestor::node[@cat="smain"]/
                 node[@rel="hd"]/%b% 
               > node[%headrel%]/%b% 
       or 
       ancestor::node[@cat="smain"]/
                 node[@rel="hd"]/%b% 
               > %b% and @pos
    )
    """

## Tools

### Using tools

In both the Tree and Sentences tab the individual files matching your
filter query are shown. You can add additional entries to the context
menu for these files by configuring tools. For example, you could
configure a text editor to show the raw XML data of the selected
sentence.

If you have selected multiple files, Dact will start the selected tool
once for each file.

### Configuring tools

The path to a tools configuration file can be set in the Preferences of
Dact. Once the path is set, the file is loaded when you open the tools
context menu. If you update the file after that, Dact will reload it
automatically.

### Tools configuration file syntax

The syntax of the tools configuration file looks a lot like the one used
by the macros. In addition to this, you can use `%1` as placeholder for
the filename of the selected file as shown by Dact in the Tree tab. For
example, to define an "Edit Text" menu entry:

    Edit Text = """gedit "~/treebanks/cdb/%1"""

> **Tip**
>
> In this example the complete path is surrounded by quotes, as the
> filename may contain spaces.

Unfortunately, the XML files Dact uses are not accessible outside of the
Dact database files. But if you have the raw XML files installed
somewhere (e.g. in `~/treebanks/cdb`), you can refer to those.

### Tools for a specific treebank

Because you cannot let the tools interact with the treebank, you might
need to configure your tools differently for each treebank you use. E.g.
your `cdb.dact` might need to use the files in `~/treebanks/cdb` while
you have all your XML files for `wikipedia.dact` in
`/net/shared/treebanks/wikipedia`.

To allow you to specify a tool for just a few specific treebanks, you
can add subsections to your tools configuration file with the path of
the treebank files as name. Fortunately, you can use wildcards in these
filenames. Only when the path of the currently loaded treebank matches
this pattern, all tools in this subsection will be shown in the tools
context menu.

    mark = """bash -c "echo \"%1\" >> ~/marked_entries.txt" """

    [*/cdb.dact]
    show xml = """gedit "~/treebanks/cdb/%1""""
    email name = """bash -c "echo \"%1\" | sendmail -v someone@domain" """

    [*/wikipedia*.dact]
    show xml = """gedit "/net/shared/treebanks/wikipedia/%1""""

## Common errors

### The application failed to initialize properly (0xc0000022)

If you get the error
`The application failed to initialize properly (0xc0000022)` it means
that something is wrong with the DLL permissions. Presumably with the
.dll files in the Dact distribution.

You can fix this problem by opening a command prompt, and cd to the
directory in which the Dact files reside. Then type:

    cacls *.dll /E /G BUILTIN\Users:R

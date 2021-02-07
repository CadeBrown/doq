# doq

Doq is a documentation system meant to target multiple output formats (markdown, HTML, PDF, and so forth). 


## Usage

To build a specific documentation, run `doq [input] [output]`, where `input` is a `.doq` file, and `output` depends on the format.




## Building

To build the project, simply clone it or download a release, then run `make` in the main directory. Only requirements are a C++ compiler

That should create the `./doq` binary that can be ran to generate documentation


## Doq Language

In general, the actual documentation should be present in a single `.doq` file. It may reference assets (such as images, audio, and video) in other places.

It is macro-based, and macros are evaluated by using the `@` character. For example, to create a list of elements, you can use `@list`:

```
@list A, B, C
```

(arguments are seperated by `,`)

As you might imagine, `,` might be in your text already. You can group arguments in `{}` to avoid parsing commas within them. For example:

```
@list {A, which is a letter}, {B, which is also a letter}, {C, yet again, a letter}
```

The above list only has 3 elements.

### Formats

A few builtin formatting tools (similar to markdown) are included in kscript.

  * `` ``` `` denote code blocks
  * `` ` `` denote inline monospace blocks
  * `$<name>` generates a reference to `<name>`, which is in the project. Equivalent to `@ref <name>`

### Builtin Macros

There are a number of builtin macros that can be used to generate platform-independent results:

  * `@today`: Returns the current date as `YYYY-MM-DD` (example: `1970-01-01`)
  * `@node <name>, <desc>, <content>...`: Creates a new node (aka page) with the given name, description, and content. Nodes can be created inside other nodes to create a tree-view
  * `@math <args>...`: Interprets `<args>...` as LaTeX-style math content
  * `@bold <args>...`: Makes `<args>...` bold
  * `@italic <args>...`: Makes `<args>...` italic
  * `@underline <args>...`: Makes `<args>...` underline
  * `@mono <args>...`: Makes `<args>...` monospace
  * `@url <url>, <content>...`: Makes `<content>...` a clickable link to `<url>`. If `<content>...` is empty, it is the URL text exactly
  * `@ref <id>, <content>...`: Makes `<content>...` a clickable link to `<id>` (a reference to another node in the project). If `<content>...` is empty, it is the id text exactly
  * `@list <args>...`: Creates an un-numbered list with `<args>...` as the elements
  * `@dict <args>...`: Creates an dictionary with `<args>...` as the keys and values (even elements are keys, odd elements are values)


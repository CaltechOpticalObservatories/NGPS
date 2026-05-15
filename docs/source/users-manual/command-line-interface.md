# Command Line Interface

A command line interface gives users the ability to send interactive commands and to create scripts to control any aspect of the instrument. A single wrapper,

`seq`

```{figure} assets/page_026_image_01.png
:alt: NGPS manual figure
:width: 90%

```

can be entered into any shell and provides the interface for both scripts and single interactive commands. The general syntax structure is

`seq <command> seq <command> <key>=<value> seq <command> <key1>=<value1> <key2>=<value2> ...`

Scripts can be made using any plain text editor and have no restrictions on the file name. Scripting syntax uses the following rules:

```text

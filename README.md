# Formatter

## Description
Formatter is a command-line program that helps users rename all the file in a
directory. The user needs to enter a pattern to search and a replacement
string. All the matched pattern in the filenames will be replaced by the
replacement string.

## Use Cases
The user can enter space for pattern, and underscore(\_) for replacement.
Filename "This is a file" will be changed to "This\_is\_a\_file". This will take
effect on all the files in the selected directory.

The user can further use wildcard characters to have more flexibility. The dot
wildcard can represent a single character. All the dots in the replacement
string matches the dots in pattern one by one. This means that the first dot in
the replacement will be replaced by the corresponding letter that first dot in
pattern matches to. For example, for a file named "Writing Assignment 2" with
pattern "Writ..." and replacement "Read...", the result file name would be
"Reading Assignment 2". The user can also use wildcard character Klenee
star(\*). It can represent 0 or more characters. Same as dot, each star in
replacement corresponds to a star in pattern with the same order. For example,
filename "Lecture_Notes_180502.pdf" with pattern "\*.......pdf" and replacement
"......\_\*.pdf" will be reanmed as "180502\_Lecture\_Notes.pdf".

The user can run the program with command line argument -x so that all the
rename won't take effect. The user can also supply a -d option with a directory
path so that the program will rename all the files in that path. 

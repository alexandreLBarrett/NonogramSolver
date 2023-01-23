# Nonogram solver

The premise of this program is to solve nonogram puzzle.

If a puzzle has multiple solutions the algorithm gives the last certain state of the puzzle.

## puzzle file format

```
<width> <height>
<column 1>
...
<column width>
<row 1>
...
<row height>
```

see files in puzzles/ for examples

## Fetching puzzles

Simply run this command in the dev console on the [logimage](https://fr.goobix.com/jeux-en-ligne/nonograms) website

```
	test = document.querySelectorAll("[id^=vert]").length + " " 
		+ document.querySelectorAll("[id^=horiz]").length + "\n"
	document
		.querySelectorAll(".nonogramsDef")
		.forEach(x => test += x.innerText.trim().replace(/(\s+|\n)/g, " ") + '\n')
	test = test.slice(0, -1)
	console.log(test.substr(0, test.lastIndexOf('\n')))
```

The width and height still need to be put by hand
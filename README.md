# lua-string-format

[![test](https://github.com/mah0x211/lua-string-format/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-string-format/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/mah0x211/lua-string-format/branch/master/graph/badge.svg)](https://codecov.io/gh/mah0x211/lua-string-format)

formatted output conversion module with asprintf and snprintf.


## Installation

```
luarocks install string-format
```


## Usage

```lua
local format = require('string.format')

-- To print a date and time in the form “Sunday, July 3, 10:02”, where weekday and month are strings
local weekday, month, day, hour, min = 'Sunday', 'July', 3, 10, 2
local s = format('%s, %s %d, %.2d:%.2d\n', weekday, month, day, hour, min)
print(s) --> Sunday, July 3, 10:02

-- To print pi to five decimal places:
s = format('pi = %.5f\n', 4 * math.atan(1.0))
print(s) --> pi = 3.14159
```


## s, unused = format( fmt [, ... ] )

converts the specified arguments to formatted output using `asprintf` and `snprintf`, and returns the result string and unused arguments.

the format `fmt` specifiers are the same as `snprintf` of the C standard library except for the following specifiers.

- flags: `#`, `0`, `-`, `+`, `space`
- width: `number`, `*`
- precision: `number`, `*`
- length: `hh`, `h`, `l`, `ll`, `j`, `z`, `t`, `L`
- specifiers: `d`, `i`, `o`, `u`, `x`, `X`, `e`, `E`, `f`, `F`, `g`, `G`, `a`, `A`, `c`, `s`, `p`, `q`, `m`, `%`
    - the format specifier `s` converts the argument to a string.
    - the format specifier `q` converts the argument to a string and escaping the control characters and double quotes `"` with a backslash `\`, and then enclosing it in double quotes `"`.

please see the manual page of `man 3 printf` for more information.


**Parameters**

- `fmt:string`: the format string that describes the format of the output.
- `...:any`: the arguments to be converted to formatted output according to the format string.

**Returns**

- `s:string`: the formatted output string.
- `unused:table?`: the unused arguments placed in the table.


## License

MIT License


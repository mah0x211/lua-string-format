local assert = require('assert')
local format = require('string.format')
local alltests = {}
local testcase = setmetatable({}, {
    __newindex = function(_, k, v)
        assert(not alltests[k], 'duplicate test name: ' .. k)
        alltests[k] = true
        alltests[#alltests + 1] = {
            name = k,
            func = v,
        }
    end,
})

function testcase.no_format()
    -- test that format() returns a string
    local s, unused = format('hello world')
    assert.equal(s, 'hello world')
    assert.is_nil(unused)
end

function testcase.character_format()
    -- test that character type: c
    local s, unused = format("%-3c", 'A')
    assert.match(s, "A  ")
    assert.is_nil(unused)

    -- test that can be used with integer
    s = format("%c", 65)
    assert.match(s, "A")

    -- test that throw error if string length is greater than 1
    local err = assert.throws(format, "%c", 'AB')
    assert.re_match(err, "bad argument #2 .+string length <=1 expected")

    -- test that throw error if argument is not a string or integer
    err = assert.throws(format, "%c", true)
    assert.re_match(err, "bad argument #2 .+number expected")
end

function testcase.string_format()
    -- test that string type: s
    for _, v in ipairs({
        {
            arg = 'world',
            expected = '^hello world$',
        },
        {
            arg = 1,
            expected = '^hello 1$',
        },
        {
            arg = 1.23,
            expected = '^hello 1.23$',
        },
        {
            arg = true,
            expected = '^hello true$',
        },
        {
            arg = false,
            expected = '^hello false$',
        },
        {
            arg = nil,
            expected = '^hello nil$',
        },
        {
            arg = {},
            expected = '^hello table: 0x[0-9a-f]+$',
        },
        {
            arg = setmetatable({}, {
                __tostring = function()
                    return 'world'
                end,
            }),
            expected = 'hello world',
        },
    }) do
        local s, unused = format('hello %s', v.arg)
        assert.re_match(s, v.expected)
        assert.is_nil(unused)
    end

    -- test that return formatted string and unused arguments
    local s, unused = format('hello %p', 'error', 'world')
    assert.re_match(s, 'hello (\\(nil\\)|0x[0-9a-f]+)')
    assert.equal(unused, {
        'world',
    })
end

function testcase.quoted_string_format()
    -- test that quoted string type: q
    for _, v in ipairs({
        -- 1 byte sequences
        {
            arg = string.char(0x40),
            expected = '"aあ@foo"',
        },
        {
            arg = string.char(0x80),
            expected = '"aあ�foo"',
        },
        -- 2 byte sequences
        {
            arg = string.char(0xC2, 0xA9),
            expected = '"aあ©foo"',
        },
        {
            arg = string.char(0xC2, 0x40),
            expected = '"aあ�@foo"',
        },
        {
            arg = string.char(0xC2, 0xC0),
            expected = '"aあ�foo"',
        },
        -- 3 byte sequences started with 0xE0
        {
            arg = string.char(0xE0, 0xA0, 0x80),
            expected = '"aあࠀfoo"',
        },
        {
            arg = string.char(0xE0, 0x40, 0x40),
            expected = '"aあ�@@foo"',
        },
        {
            arg = string.char(0xE0, 0xA0, 0x40),
            expected = '"aあ�@foo"',
        },
        {
            arg = string.char(0xE0, 0xA0, 0xC0),
            expected = '"aあ�foo"',
        },
        -- 3 byte sequences started with 0xE1-EC
        {
            arg = string.char(0xE1, 0xB4, 0x81),
            expected = '"aあᴁfoo"',
        },
        {
            arg = string.char(0xE1, 0x40, 0x40),
            expected = '"aあ�@@foo"',
        },
        {
            arg = string.char(0xE1, 0xB4, 0x40),
            expected = '"aあ�@foo"',
        },
        {
            arg = string.char(0xE1, 0xB4, 0xC0),
            expected = '"aあ�foo"',
        },
        -- 3 byte sequences started with 0xED
        {
            arg = string.char(0xED, 0x80, 0x80),
            expected = '"aあ퀀foo"',
        },
        {
            arg = string.char(0xED, 0x40, 0x40),
            expected = '"aあ�@@foo"',
        },
        {
            arg = string.char(0xED, 0x80, 0x40),
            expected = '"aあ�@foo"',
        },
        {
            arg = string.char(0xED, 0x80, 0xC0),
            expected = '"aあ�foo"',
        },
        -- 3 byte sequences started with 0xEE-EF
        {
            arg = string.char(0xEF, 0xA4, 0x80),
            expected = '"aあ豈foo"',
        },
        {
            arg = string.char(0xEF, 0x40, 0x40),
            expected = '"aあ�@@foo"',
        },
        {
            arg = string.char(0xEF, 0xA4, 0x40),
            expected = '"aあ�@foo"',
        },
        {
            arg = string.char(0xEF, 0xA4, 0xC0),
            expected = '"aあ�foo"',
        },
        -- 4 byte sequences started with 0xF0
        {
            arg = string.char(0xF0, 0x90, 0x82, 0x82),
            expected = '"aあ𐂂foo"',
        },
        {
            arg = string.char(0xF0, 0x40, 0x40, 0x40),
            expected = '"aあ�@@@foo"',
        },
        {
            arg = string.char(0xF0, 0x90, 0x40, 0x40),
            expected = '"aあ�@@foo"',
        },
        {
            arg = string.char(0xF0, 0x90, 0x82, 0x40),
            expected = '"aあ�@foo"',
        },
        {
            arg = string.char(0xF0, 0x90, 0x82, 0xC0),
            expected = '"aあ�foo"',
        },
        -- control sequences
        {
            arg = '\a\0\b\t\n\v\f' .. string.char(14) .. '\r' .. string.char(15) ..
                '987',
            expected = '"aあ\\a\\0\\b\\t\\n\\v\\f\\14\\r\\015987foo"',
        },
        -- double-quote or backslash will be escaped
        {
            arg = '"\\',
            expected = '"aあ\\"\\\\foo"',
        },
    }) do
        local s = format("%q", 'aあ' .. v.arg .. 'foo')
        assert.equal(s, v.expected)
    end

    -- test that throw error if %q with modifier
    local err = assert.throws(format, "%-3q", 'a')
    assert.re_match(err, "'%q' cannot have modifiers")
end

function testcase.integer_format()
    -- test that integer type: d, i, o, u, x, X
    local s, unused = format('%+d %-5i %05o %u %#x %#X %ld %d %d', 42, 42, 42,
                             42, 42, 42, 42, true, false)
    assert.equal(s, "+42 42    00052 42 0x2a 0X2A 42 1 0")
    assert.is_nil(unused)
end

function testcase.float_format()
    -- test that floting point type: e, E, f, F, g, G
    local s, unused = format("%+e %-.*E %+f % F %.1g %.1G", 1.23, 2, 1.23, 1.23,
                             1.23, 1.23, 1.23)
    assert.match(s, "+1.230000e+00 1.23E+00 +1.230000  1.230000 1 1")
    assert.is_nil(unused)

    -- test that floating point in hexdigit: a, A
    s = format("%a %#A", 1.23, 1.23)
    assert.match(s, "0x1%.[a-f0-9p+]+ 0X1%.[A-F0-9P+]+", false)
end

function testcase.pointer_format()
    -- test that pointer type: p
    local s, unused = format("%p", {})
    assert.match(s, "0x[0-9a-f]+", false)
    assert.is_nil(unused)
end

function testcase.escape_format()
    -- test that escape: %
    local s, unused = format("%%")
    assert.match(s, "%")
    assert.is_nil(unused)
end

function testcase.error_format()
    -- test that print errno: m
    local s, unused = format("%m")
    assert(s ~= nil)
    assert.is_nil(unused)
end

function testcase.unsupported_format()
    -- test that throw error if unsupported format type is specified
    local err = assert.throws(format, "%V")
    assert.match(err, "unsupported type field")
end

local gettime = require('time.clock').gettime
local stdout = io.stdout
local elapsed = gettime()
local errs = {}
print(string.format('Running %d tests...\n', #alltests))
for _, test in ipairs(alltests) do
    stdout:write('- ', test.name, ' ... ')
    local t = gettime()
    local ok, err = pcall(test.func)
    t = gettime() - t
    if ok then
        stdout:write('ok')
    else
        stdout:write('failed')
        errs[#errs + 1] = {
            name = test.name,
            err = err,
        }
    end
    stdout:write(' (', string.format('%.2f', t), ' sec)\n')
end
elapsed = gettime() - elapsed
print('')
if #errs == 0 then
    print(string.format('%d tests passed. (%.2f sec)\n', #alltests, elapsed))
    os.exit(0)
end

print(string.format('Failed %d tests:\n', #errs))
local stderr = io.stderr
for _, err in ipairs(errs) do
    stderr:write('- ', err.name)
    stderr:write(err.err, '\n')
end
print('')
os.exit(-1)

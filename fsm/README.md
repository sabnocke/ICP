# Syntax
- Any keyword is case-insensitive
- There are no comments, beside `comment:`
- No section is required, but for the automat to work correctly, 
 _states_ and _transitions_ are **required** and *signals* with *variables* are **optional**

## Name
- `name(:) <name>`
- The colon is optional

## Comment
- Ideally should be on single line
- `comment(:) <comment>`
- The colon is optional

## Signals
- `inputs: <name>, <name1>, ...`
- `inputs` can also be `outputs`

## Variables
- `<type> <name> = <value>`
- type should be one of
  - int
  - double
  - string
  - bool
- all variables must be initialized (assigned a value)
- lua on its own doesn't have types, so these act more as a constraint

## States
- `state <name> [<action>]`
- _action_ can be empty and can be on multiple lines
  - Additionally, _action_ can be anything that basic lua can compile or uses predefined functions
  - These are: `valueof(name)`, `defined(name)` or `output(name, value)`
- _name_ should be unique
- automat doesn't verify reachability of any state

## Transitions
- Whole section needs to start with `Transitions:` line (maybe remove that?)
- `<from> --> <to>: <input>? [<condition>]? @ <delay>?`

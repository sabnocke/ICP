Outputs = {}
Inputs = {}
-- TimeElapsed = 0.0

function output(name, value)
    Outputs[name] = value
    return name .. " = " .. value
end

function defined(name) return Inputs[name] ~= nil end

function valueof(name) return Inputs[name] end

-- will be provided by sol
-- function elapsed() return TimeElapsed end
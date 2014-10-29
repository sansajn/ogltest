-- ukazka resource systemu pre tweakbar (napísaného v lue)

-- pomocne funkcie (by bol zapis human-readable)
function editor(opts)
	return opts
end

function tweakbar(opts)
	opts['type'] = 'tweakbar'
	return opts
end

function tweakscene(opts)
	opts['type'] = 'tweakscene'
	return opts
end

function float(opts)
	opts['type'] = 'float'
	return opts
end

function tweakbar_manager(opts)
	opts['type'] = 'tweakbar_manager'
	return opts
end


-- v samotnom configu bude iba toto
content = tweakbar_manager{
	name='twekBar', 
	next='viewHandler',

	editor{  -- 1
		id='resources', permanent=true, exclusive=false, 
		tweakbar{  -- 1
			float{label='Exposure', keyincr='+', keydecr='-', group='Rendering', 
				min=0.01, max=10.0, step=0.01}
		}
	},

	editor{  -- 2
		id='scene', exclusive=false, 
		tweakscene{  -- 1
			name='editScene', scene='scene', active=false}
	},	
}  -- tweakbar_manager



function print_table(t)
	for k,v in pairs(t) do
		if type(v) == 'table' then
			print_table(v)
		else
			print(k,v)
		end
	end
end

function get_content()
	return content
end

-- print_table(content)

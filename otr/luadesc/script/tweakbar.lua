function tweakbar(opts)
	opts.type = 'tweakbar'
	return opts
end

function float(opts)
	opts.type = 'float'
	return opts
end

function bool(opts)
	opts.type = 'bool'
	return opts
end

content = tweakbar{name='settings', label='Options',
	float{label='Exposure', keyincr='+', keydecr='-', group='Rendering', 
		help='help ...', min=0.01, max=10.0, step=0.01
	},

	bool{label='State', group='Rendering'},
}

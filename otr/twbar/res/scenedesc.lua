function tweakbar(args)
	args.type='tweakbar'
	return args
end

function tweakbar_manager(args)
	args.type='tweakbar_manager'
	return args
end

function editor(args)
	args.type='editor'
	return args
end

function float(args)
	args.type='float'
	return args
end

function drawtweakbar(args)
	args.type='drawtweakbar'
	return args
end

-- modules
camera = module{version=330, source='camera.glsl'}
plastic = module{version=330, source='plastic.glsl'}

-- methods
camera_method = sequence{
	foreach{var='o', flag='object', call{'$o.draw'}},
	foreach{var='o', flag='overlay', call{'$o.draw'}}
}

object_method = sequence{
	setprogram{
		module{name='camera.material'}, 
		module{name='this.material'}
	},
	transforms{local_to_screen='local_to_screen', local_to_world='local_to_world'},
	drawmesh{name='this.geometry'}
}

tweakbar_method = drawtweakbar{}

scene = node{
	node{  -- camera
		name='camera',
		flags='camera',
		module{id='material', value=camera},
		method{id='draw', value=camera_method}
	},
	node{  -- model
		name='model',
		flags='object',
		mesh{id='geometry', value='monkey.ply'},
		module{id='material', value=plastic},
		method{id='draw', value=object_method}
	},
	node{  -- tweakbar
		flags='overlay',
		method{id='draw', value=tweakbar_method}
	}
}

twbar_render = tweakbar{
	label='Options',
	float{
		label='Exposure',
		group='Rendering',
		keyincr='+', keydecr='-',
		min=0.1, max=1.0, step=0.05,
		path='camera/material/color_coef'
	}
}

twbar_man = tweakbar_manager{
	editor{id='render', bar=twbar_render}
}

-- app = window{ui=twbar_man, view=view_handler}


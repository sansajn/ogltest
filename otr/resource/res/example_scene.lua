-- prepis exampleScene.xml (bez osvetlenia)
function foreach(args)
	args.type = 'foreach'
	return args
end

function call(args)
	args.type = 'callmethod'
	return args
end

function node(args)
	args.type = 'node'
	return args
end

function method(args)
	args.type = 'method'
	return args
end

function translate(args)
	args.type = 'translate'
	return args
end

function rotate(args)
	args.type = 'rotate'
	return args
end

function bounds(args)
	args.type = 'bounds'
	return args
end

function uniform3f(args)
	args.type = 'uniform3f'
	return args
end

function module(args)
	args.type = 'module'
	return args
end

function mesh(args)
	args.type = 'mesh'
	return args
end

function depth(args)
	args.type = 'depth'
	return args
end

function state(args)
	args.type = 'state'
	return args
end

function transforms(args)
	args.type = 'transforms'
	return args
end

function sequence(args)
	args.type = 'sequence'
	return args
end

function set_program(args)
	args.type = 'setprogram'
	return args
end

function drawmesh(args)
	args.type = 'drawmesh'
	return args
end

function showlog(args)
	args.type = 'showlog'
	return args
end

function showinfo(args)
	args.type = 'showinfo'
	return args
end

function texture2D(args)
	args.type = 'texture2D'
	return args
end

function uniform_sampler(args)
	args.type = 'uniform_sampler'
	return args
end


-- methods
camera_method = sequence{
--	state{depth{enable=true, value='LESS'}},
	transforms{module='this.material', worldpos='world_camera_pos'},
	foreach{var='l', flag='light', call{'$l.draw'}},
	foreach{var='o', flag='object', culling=true, call{'$o.draw'}},
	foreach{var='o', flag='overlay', call{'$o.draw'}}
}

object_method = sequence{
	set_program{
		module{name='camera.material'},
--		module{name='light.material'},
		module{name='this.material'}},
	transforms{loacal_to_screen='local_to_screen', local_to_world='local_to_world'},
	drawmesh{name='this.geometry'}
}

--log_method = showlog{x=4, y=-4, maxlines=32, fontsize=16, fontprogram='text;'}
--info_method = showinfo{x=4, y=4, maxlines=4, fontsize=24, fontprogram='text;'}

-- textures
checker = texture2D{source='checker.png', internal_format='RGB8', min='LINEAR_MIPMAP_LINEAR', mag='LINEAR'}

-- modules
camera = module{version=330, source='camera.glsl'}
plastic = module{version=330, vertex='plastic_vs.glsl', fragment='plastic_fs.glsl'}

textured_plastic = 
	module{version=330, source='textured_plastic.glsl',
		uniform_sampler{texture=checker}}

-- nodes
scene = node{
	node{flags='camera',
		module{id='material', value=camera},
		method{id='draw', value=camera_method}
	},

	node{flags='object',
		rotate{0, 0, 15},
		mesh{id='geometry', value='cube.mesh'},
		module{id='material', value=plastic},
		method{id='draw', value=object_method}
	},

	node{flags='object',
		translate{0, 0, -2},
		rotate{0, 0, 180},
		mesh{id='geometry', value='plane.mesh'},
		module{id='material', value=textured_plastic},
		method{id='draw', value=object_method}
	},

--	node{flags='overlay',
--		method{id='draw', value=log_method}
--	},

--	node{flags='overlay',
--		method{id='draw', value=info_method}
--	}
}

print(scene)

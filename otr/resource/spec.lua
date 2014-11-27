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



-- test specifikacie
camera_method = 
	foreach{var='o', flag='dynamic', parallel=true,	call{'$o.draw'}}

plastic = module{version=330, source='shaders/plastic.glsl', options='option1,debug'}

scene = node{
	node{flags='camera', 
		method{id='draw', value=camera_method}
	},

	node{
		--translate{10,0,-5},
		--rotate{0,0,15},
		--bounds{xmin=-1, xmax=1, ymin=-1, ymax=1, zmin=-1, zmax=1},
		--uniform3f{id='ambient', name='ambient_color', {0, 0.1, 0.4}},
		module{id='material', value=plastic},
		--mesh{id='geometry', value='cube.mesh'},
		--method{id='draw', value=camera_method}
	}
}  -- scene


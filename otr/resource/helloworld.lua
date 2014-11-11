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

camera_method = 
	foreach{var='o', flag='dynamic', parallel=true,	call{'$o.draw'}}

scene = node{
	node{flags='camera', 
		method{id='draw', value=camera_method}}
}  -- scene


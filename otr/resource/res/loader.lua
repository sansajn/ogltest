-- elements support
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

function setprogram(args)
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

function texture2d(args)
	args.type = 'texture2d'
	return args
end

function uniform_sampler(args)
	args.type = 'uniform_sampler'
	return args
end

dofile(__scenefile)


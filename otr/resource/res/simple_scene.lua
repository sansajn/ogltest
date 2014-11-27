-- modules
camera = module{version=330, source='camera.glsl'}
plastic = module{version=330, source='plastic.glsl'}

-- methods
camera_method = sequence{
	transforms{module='this.material', worldpos='world_camera_pos'},
	foreach{var='o', flag='object', call{'$o.draw'}}
}

object_method = sequence{
	setprogram{
		module{name='camera.material'},
		module{name='this.material'}
	},
	transforms{local_to_screen='local_to_screen', local_to_world='local_to_world'},
	drawmesh{name='this.geometry'}
}

scene = node{
	node{
		flags='camera',
		module{id='material', value=camera},
		method{id='draw', value=camera_method}
	},
	node{
		flags='object',
		mesh{id='geometry', value='cube.mesh'},
		module{id='material', value=plastic},
		method{id='draw', value=object_method}
	}
}

no v podstate bez loaderu je resource manager iba mapa, ktorá asociuje zdroj s
klúčom.


Ten loader neloaduje zdroje sekvenčne zo súboru, ale skrz resource-manager si
aplykácia požiada napr. o 'scene', čo vyústi k prečítaniu uzla 'scene'. Pri
vytvárani uzla sa zistí, že je potrebné vytvoriť metódu 'camera_method' a
resource manager sa požiada o 'camera_method'.

[Ukážka:

	archive{
		sequence{name='camera_method',                  -- 1
			for{var='o', flag='dynamic', parallel=true,  -- 1.1
				call{name='$o.draw'}                      -- 1.1.1
			}
		},

		node{name='scene',                              -- 2
			node{flags='camera',                         -- 2.1
				method{id='draw', value='camera_method'}  -- 2.1.1
			},
		}  -- scene
	}  -- archive

--- koniec ukážky].

Takéto vytváranie zdrojov na požidanie je ideálne pre zásobník (ako na napr.
lua).

Pri takejto štruktúre sa dá explicitne odkázať iba na prvky v roote s atributom
name (lua poskytuje možnosť odkázať sa na hocktorý prvok) napr. 'camera_method'
a 'scene'.

Lua script mi môže vytvoriť mapu prvkou s name atribútom, alebo môže definovať
funkciu 'find(name)'.


Ak by som mal takýto popis

	camera_method = sequence{
		for{var='o', flag='dynamic', parallel=true,
			call{name='$o.draw'}
		}}

	archive{
		node{name='scene',
			node{flags='camera',
				method{id='draw', value=camera_method}		
			}  -- camera
		}  -- scene
	}

premenná camera_method hrá rovnakú úlohu ako reťazcový identifikátor
'camera_method'. Teraz som ale stratil možnosť explicitne vytvoriť zdroj s
názvom 'camera_method'.


Zdá sa, že používanie name argumentou je možné nahradiť premennými, takto
[Ukážka:

	camera_method = 
		for{var='o', flag='dynamic', parallel=true,
			call{name='$o.draw'}
		}

	scene = node{
		node{flags='camera', 
			method{id='draw', value=camera_method}}
	}  -- scene

--- koniec ukážky]. Aplikácia si z globálnej tabuľky vyžiada premennú daného
nazvu napr. [Ukážka:

	// vyžiadaj si premennú scene

--- koniec ukážky] a deskriptor bude jednoducho identifikátor premennej.

Kompletná scéna [Ukážka:

	default_scheduler = multithread_scheduler{nthreads=3, fps=0}

	camera_method = {
		for{var='o', flag='dynamic', parallel=true, 
			call{'$o.update'}},
		for{var='o', flag='object', culling=true, 
			call{'$o.draw'}}
	}

	terrain = terrain_node{size=500, zmin=-1, zmax=1, split_factor=2, max_level=7}

	update_terrain_method = update_terrain{'this.terrain'}

	draw_terrain_method = {
		program{module{'this.material'}},
		draw_terrain{name='this.terrain', mesh='this.grid', culling=true}
	}

	terrain_shader = module{version=330, src='terrain_shader.glsl'}

	scene = node{
		node{flags='camera',
			method{id='draw', value=camera_method}
		},

		node{flags={'object', 'dynamic'},
			bounds{xmin=-500, xmax=500, ymin=-500, ymax=500, zmin=-1, zmax=1},
			field{id='terrain', value=terrain},
			mesh{id='grid', value='quad.mesh'},
			method{id='update', value=update_terrain_method},
			method{id='draw', value=draw_terrain_method},
			module{id='material', value=terrain_shader}
		}
	}  -- scene

--- koniec ukážky], kde sa program na zroje môže odkazovať identifikátorom
premennej (napr. scene, terrain, draw_terrain_method, ...). Všetky premenné
prdstavujú adresovateľné zdroje.








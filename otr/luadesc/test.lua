function test()
	print('lua:test()')
	return 'Frenk', 22, 87.5
end

function person_data(name)
	print('lua:person_data(name=' .. name .. ')')
	if name == 'Frenk Black' then
		return 25, 1890
	else
		return -1, -1
	end
end

function dummy_table()
	print('lua:dummy_table()')
	return {1, 2, 3, 2, 1}
end

function simple_error()
	f:write('write somthing somewere')
end

function filter(d)
	fd = {}
	for k,v in ipairs(d) do
		fd[k] = v*v;
	end
	return fd
end

function write_test()
	f = assert(io.open('testfile.dat', 'w'))
	f:write('Hello!')
	f:close()
end

function sqlite3_test()
	require 'luasql.sqlite3'

	env = assert(luasql.sqlite3())
	con = assert(env:connect('test.db'))
	res = assert(con:execute[[
		create table people(
			name varchar(50),
			email varchar(50)
		)
	]])

	list = {
		{name='Jose das Couves', email='jose@couves.com'},
		{name='Manoel Joaqim', email='manuel@joaqim.com'},
		{name='Maria das Dores', email='maria@dores.com'}
	}

	for i,v in pairs(list) do
		res = assert(con:execute(string.format([[
			insert into people
			values('%s', '%s')]], v.name, v.email)
		))
	end

	cur = assert(con:execute("select name, email from people"))
	row = cur:fetch({}, 'a')
	while row do
		print(string.format('Name: %s, E-mail: %s', row.name, row.email))
		row = cur:fetch(row, 'a')
	end

	cur:close()
	con:close()
	env:close()
end

function echo_boolean(b)
	return b
end

function echo_table(t)
	return t
end

function custom_structure_test(person)
	if person.name == 'Franta Spelec' and person.age == 29 then
		return 1
	else
		return 0
	end
end

function custom_structure_array_test(data)
	expected = {
		{name='Franta Spelec', age=29},
		{name='John Rambo', age=33},
		{name='Janosikov sveter', age=26}
	}

	for i,v in pairs(data) do
		if v.name ~= expected[i].name or v.age ~= expected[i].age then
			return 0
		end
	end

	return 1
end

function ostream_binary_test(data)
	require 'pack'
	
	if #data ~= 15 then
		return -1
	end

	local packed = string.pack('>I', 12345)

	local sum = 0
	for i=1,#data do
		local len, val = string.unpack(data, 'b', i)
		sum = sum + val
	end

	return sum
end

function return_table()
	return {name='Peter', age=33}
end

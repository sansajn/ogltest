in VS_OUT {
	vec4 color;
} fs_in;

out vec4 fcolor;

void main()
{
	fcolor = fs_in.color;
}

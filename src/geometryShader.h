#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

in VS_OUT {
    vec3 color;
} gs_in[];

out vec3 fColor;

void build_house(vec4 position)
{    
	int n = 5;
	float pi = 3.14;
    fColor = gs_in[0].color; // gs_in[0] since there's only one input vertex
	for(int i=0;i<n;i++){
		gl_Position = position + vec4(0.2 * sin(pi/i), 0.5 * cos(pi/i), 0.0, 0.0); // 1:bottom-left   
		EmitVertex();
	}    
    EndPrimitive();
}

void main() {    
    build_house(gl_in[0].gl_Position);
}
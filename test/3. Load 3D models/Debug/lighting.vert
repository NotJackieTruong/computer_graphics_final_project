
uniform vec3 Eye; // eye position
uniform vec3 lightPos; // lightDir vector
uniform vec3 lightColor; // s_d

uniform float materialSh; // sh		// we set uniform variables in the OpenGL code.


out vec4 oColor;//pervertex Varying variables provide an interface between Vertex and Fragment Shader.

void main()
{	
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	vec3 vPos = (gl_ModelViewMatrix * gl_Vertex).xyz;
	vec3 vNormal = normalize(gl_NormalMatrix * gl_Normal).xyz;
	vec3 vColor = vec3(1.0, 1.0, 1.0);//  gl_Color.xyz;

	vec3 lightDir = normalize(lightPos - vPos);
	// Diffuse
	float diffuse = max(dot(vNormal, lightDir), 0.0f);

	// Specular
	vec3 Refl = 2.0f*vNormal*dot(vNormal, lightDir) - lightDir;
	vec3 viewDir = normalize(-vPos);
	float specular = pow(max(dot(Refl, viewDir), 0.0f), materialSh);
	if (diffuse <= 0.0f) specular = 0.0f;

	// Ambient
	float ambient = 0.0;

	// Emissive
	float emissive = 0.0;

	// Sum
	vec3 color = vec3(diffuse + specular + ambient + emissive)*lightColor*vColor;
	oColor = vec4(color, 1.0f);
}
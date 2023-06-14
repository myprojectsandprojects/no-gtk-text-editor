#include <stdio.h>
#include "opengl.h"

GLuint make_shader(const char *VertexSrc, const char *FragmentSrc, const char *GeometrySrc/* = NULL*/)
{
	int success;
	char info_log[512];

	GLuint VertexShader   = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(VertexShader, 1, &VertexSrc, NULL);
	glShaderSource(FragmentShader, 1, &FragmentSrc, NULL);

	glCompileShader(VertexShader);
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(VertexShader, sizeof(info_log), NULL, info_log);
		printf("vertex shader error: %s\n", info_log);
		//@ error handling
	}

	glCompileShader(FragmentShader);
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(FragmentShader, sizeof(info_log), NULL, info_log);
		printf("fragment shader error: %s\n", info_log);
		//@ error handling
	}

	GLuint ShaderProgram = glCreateProgram();
	glAttachShader(ShaderProgram, VertexShader);
	glAttachShader(ShaderProgram, FragmentShader);

// "‘GL_GEOMETRY_SHADER’ was not declared in this scope" -- whats up with that?
//	if(GeometrySrc)
//	{
//		GLuint GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
//		glShaderSource(GeometryShader, 1, &GeometrySrc, NULL);
//		glCompileShader(GeometryShader);
//		glGetShaderiv(GeometryShader, GL_COMPILE_STATUS, &success);
//		if (!success)
//		{
//			glGetShaderInfoLog(GeometryShader, sizeof(info_log), NULL, info_log);
//			printf("geometry shader error: %s\n", info_log);
//		//@ error handling
//		}
//		glAttachShader(ShaderProgram, GeometryShader);
//	}

	glLinkProgram(ShaderProgram);

	//@ delete shaders?
//	free(vertexShaderText);
//	free(fragmentShaderText);

	return ShaderProgram;
}

void pass_to_shader(GLuint Shader, const char *Name, int Value)
{
	GLint UniformLocation = glGetUniformLocation(Shader, Name);
	glUniform1i(UniformLocation, Value);
}

void pass_to_shader(GLuint Shader, const char *Name, float Value)
{
	GLint UniformLocation = glGetUniformLocation(Shader, Name);
	glUniform1f(UniformLocation, Value);
}

GLuint make_color_shader()
{
	const char *VertexShader = R"(
		#version 330 core
		
		layout(location = 0) in vec2 Position;
		layout(location = 1) in vec4 vColor;
		
		out vec4 fColor;
		
		void main()
		{
			gl_Position = vec4(Position, 0.0, 1.0);
			fColor = vColor;
		}
	)";
	const char *FragmentShader = R"(
		#version 330 core
		
		in vec4 fColor;
		out vec4 Color;
		
		void main()
		{
			Color = fColor;
//			Color = vec4(0.0, 0.0, 1.0, 1.0);
		}
	)";
	return make_shader(VertexShader, FragmentShader);
}

GLuint make_color_shader_with_transform()
{
	const char *VertexShader = R"(
		#version 330 core
		
		layout(location = 0) in vec2 Position;
		layout(location = 1) in vec4 vColor;
		
		out vec4 fColor;

		uniform float WindowWidth;
		uniform float WindowHeight;
		
		void main()
		{
			gl_Position = vec4(
				Position.x * 2.0 / WindowWidth - 1.0,
				-(Position.y * 2.0 / WindowHeight) + 1.0,
				0.0,
				1.0);
			fColor = vColor;
		}
	)";
	const char *FragmentShader = R"(
		#version 330 core
		
		in vec4 fColor;
		out vec4 Color;
		
		void main()
		{
			Color = fColor;
//			Color = vec4(0.0, 0.0, 1.0, 1.0);
		}
	)";
	return make_shader(VertexShader, FragmentShader);
}

GLuint make_texture_shader()
{
	const char *VertexShader = R"(
		#version 330 core
		
		layout(location = 0) in vec3 Position;
		layout(location = 1) in vec2 vTex;
		
		out vec2 fTex;
		
		void main()
		{
			gl_Position = vec4(Position, 1.0);
			fTex = vTex;
		}
	)";
	const char *FragmentShader = R"(
		#version 330 core
		
		in vec2 fTex;
		out vec4 Color;
		
		uniform sampler2D s;
		
		void main()
		{
			Color = texture(s, fTex);
		}
	)";
	return make_shader(VertexShader, FragmentShader);
}

GLuint make_SDF_shader()
{
	const char *VertexShader = R"(
		#version 330 core
		
		layout(location = 0) in vec3 Position;
		layout(location = 1) in vec2 vTex;
		
		out vec2 fTex;
		
		void main()
		{
			gl_Position = vec4(Position, 1.0);
			fTex = vTex;
		}
	)";
	const char *FragmentShader = R"(
		#version 330 core

		in vec2 fTex;
		out vec4 Color;

		uniform float ScaleFactor;

		uniform sampler2D s;

//		const float smoothing = 1.0 / 16.0;
//		float smoothing = 4 * ScaleFactor / 6.0;
//		float smoothing = 0.25 / (6.0 * ScaleFactor);
//		float smoothing = 1 / (12.0 * ScaleFactor);
		float smoothing = 1 / (16.0 * ScaleFactor);

		vec4 get_smooth(vec3 Color, float Distance)
		{
			return vec4(Color, smoothstep(0.5 - smoothing, 0.5 + smoothing, Distance));
		}

		vec4 get_smooth_with_background(vec4 Color, vec4 Background, float Distance)
		{
			float T = smoothstep(0.5 - smoothing, 0.5 + smoothing, Distance);
			return mix(Background, Color, T);
		}

		vec4 get_crisp(vec3 Color, float Distance)
		{
			float Alpha = (Distance < 0.5) ? 0.0 : 1.0;
			return vec4(Color, Alpha);
		}

		vec4 get_sdf(float Distance)
		{
			return vec4(1.0, 1.0, 1.0, Distance);
		}

		void main()
		{
			float Distance = texture(s, fTex).r;

//			Color = get_sdf(Distance);
//			Color = get_crisp(vec3(1.0, 1.0, 1.0), Distance);
			Color = get_smooth(vec3(1.0, 1.0, 1.0), Distance);
//			Color = get_smooth_with_background(vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 0.0, 1.0, 1.0), Distance);
		}
	)";
	return make_shader(VertexShader, FragmentShader);
}

GLuint make_text_shader()
{
	const char *VertexShader = R"(
		#version 330 core
	
		layout(location = 0) in vec2 Position;
		layout(location = 1) in vec2 vTex;
		layout(location = 2) in vec4 vColor;
	
		out vec2 fTex;
		out vec4 fColor;
	
		void main()
		{
			gl_Position = vec4(Position, 0.0, 1.0);
			fTex = vTex;
			fColor = vColor;
		}
	)";

	const char *FragmentShader = R"(
		#version 330 core
		
		in vec2 fTex;
		in vec4 fColor;
		
		out vec4 Color;
		
		uniform sampler2D s;
		
		void main()
		{
			Color = vec4(fColor.rgb, fColor.a * texture(s, fTex).r);
//			Color = vec4(1.0, 1.0, 1.0, texture(s, fTex).r);
//			Color = vec4(texture(s, fTex).r, 1.0, 1.0, 1.0);
		}
	)";

	return make_shader(VertexShader, FragmentShader);
}

GLuint make_text_shader_with_transform()
{
	const char *VertexShader = R"(
		#version 330 core
	
		layout(location = 0) in vec2 Position;
		layout(location = 1) in vec2 vTex;
		layout(location = 2) in vec4 vColor;
	
		out vec2 fTex;
		out vec4 fColor;

		uniform float WindowWidth;
		uniform float WindowHeight;
	
		void main()
		{
			gl_Position = vec4(
				Position.x * 2.0 / WindowWidth - 1.0,
				-(Position.y * 2.0 / WindowHeight) + 1.0,
				0.0,
				1.0);
			fTex = vTex;
			fColor = vColor;
		}
	)";

	const char *FragmentShader = R"(
		#version 330 core
		
		in vec2 fTex;
		in vec4 fColor;
		
		out vec4 Color;
		
		uniform sampler2D s;
		
		void main()
		{
			Color = vec4(fColor.rgb, fColor.a * texture(s, fTex).r);
//			Color = vec4(1.0, 1.0, 1.0, texture(s, fTex).r);
//			Color = vec4(texture(s, fTex).r, 1.0, 1.0, 1.0);
		}
	)";

	return make_shader(VertexShader, FragmentShader);
}

GLuint make_text_shader_geometry_style()
{
	const char *VertexShader = R"(
		#version 330 core

		layout(location = 0) in vec2 XY0;
		layout(location = 1) in vec2 XY1;
		layout(location = 2) in vec2 TXY0;
		layout(location = 3) in vec2 TXY1;
		layout(location = 4) in vec4 color;
		
		out vec2 gXY0;
		out vec2 gXY1;
		out vec2 gTXY0;
		out vec2 gTXY1;
		out vec4 gColor;

//		uniform float MyUniform;
		uniform float PixelWidth;
		uniform float PixelHeight;
		
		void main()
		{
//		float _X = X * PixelWidth - 1.0f;
//		float _Y = 1.0f - Y * PixelHeight;
//		float _Width = Width * PixelWidth;
//		float _Height = Height * PixelHeight;

//			gl_Position = vec4(upperLeft, 0.0, 1.0);
//			gXY0 = XY0;
//			gXY1 = XY1;

			gXY0.x = XY0.x * PixelWidth - 1.0;
			gXY0.y = 1.0 - XY0.y * PixelHeight;
			gXY1.x = XY1.x * PixelWidth - 1.0;
			gXY1.y = 1.0 - XY1.y * PixelHeight;

			gTXY0 = TXY0;
			gTXY1 = TXY1;
			gColor = color;
//			fColor = color;

//			gXY0.x += MyUniform;
//			gXY1.x += MyUniform;
		}
	)";

	const char *GeometryShader = R"(
		#version 330 core
		
		layout(points) in;
		layout(triangle_strip, max_vertices=6) out;
		
//		in vec2 gtexture[];

		in vec2 gXY0[];
		in vec2 gXY1[];
		in vec2 gTXY0[];
		in vec2 gTXY1[];
		in vec4 gColor[];

		//in vertexData
		//{
		//	vec4 color;
		//} GS[];
		
		out vec2 fTexture;
		out vec4 fColor;
		
		void main()
		{
			fColor = gColor[0];

			gl_Position = vec4(gXY0[0], 0.0, 1.0);
			fTexture = vec2(gTXY0[0].x, gTXY1[0].y);
			EmitVertex();

			gl_Position = vec4(gXY0[0].x, gXY1[0].y, 0.0, 1.0);
			fTexture = vec2(gTXY0[0].x, gTXY0[0].y);
			EmitVertex();

			gl_Position = vec4(gXY1[0], 0.0, 1.0);
			fTexture = vec2(gTXY1[0].x, gTXY0[0].y);
			EmitVertex();

			gl_Position = vec4(gXY0[0], 0.0, 1.0);
			fTexture = vec2(gTXY0[0].x, gTXY1[0].y);
			EmitVertex();

			gl_Position = vec4(gXY1[0].x, gXY0[0].y, 0.0, 1.0);
			fTexture = vec2(gTXY1[0].x, gTXY1[0].y);
			EmitVertex();

			gl_Position = vec4(gXY1[0], 0.0, 1.0);
			fTexture = vec2(gTXY1[0].x, gTXY0[0].y);
			EmitVertex();
		
			EndPrimitive();
		}
	)";

	const char *FragmentShader = R"(
		#version 330 core
		
		in vec4 fColor;
		in vec2 fTexture;
		
		out vec4 color;

		uniform sampler2D s;
		void main()
		{
			color = vec4(fColor.rgb, fColor.a * texture(s, fTexture));
//			color = vec4(1.0, 1.0, 1.0, 1.0);
		}
	)";

	return make_shader(VertexShader, FragmentShader, GeometryShader);
//	return make_shader(VertexShader, FragmentShader);
}
#include "drawing.hpp"

#include <assert.h>

//static shaders Shaders;
shaders Shaders;

void init_shaders(int WindowWidth, int WindowHeight)
{
	Shaders.WindowWidth = WindowWidth;
	Shaders.WindowHeight = WindowHeight;
	Shaders.ColorShader = make_color_shader_with_transform();
	Shaders.TextShader = make_text_shader_with_transform();
	Shaders.TextureShader = make_texture_shader_with_transform();
}

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

//GLuint make_color_shader()
//{
//	const char *VertexShader = R"(
//		#version 330 core
//		
//		layout(location = 0) in vec2 Position;
//		layout(location = 1) in vec4 vColor;
//		
//		out vec4 fColor;
//		
//		void main()
//		{
//			gl_Position = vec4(Position, 0.0, 1.0);
//			fColor = vColor;
//		}
//	)";
//	const char *FragmentShader = R"(
//		#version 330 core
//		
//		in vec4 fColor;
//		out vec4 Color;
//		
//		void main()
//		{
//			Color = fColor;
////			Color = vec4(0.0, 0.0, 1.0, 1.0);
//		}
//	)";
//	return make_shader(VertexShader, FragmentShader);
//}

GLuint make_color_shader_with_transform()
{
	const char *VertexShader = R"(
		#version 330 core
		
		layout(location = 0) in vec2 Position;
		layout(location = 1) in vec4 vColor;
		
		out vec4 fColor;

		uniform float WindowWidth;
		uniform float WindowHeight;

		vec2 transform_window_2_opengl(vec2 Pos)
		{
			Pos.x = Pos.x * 2.0 / WindowWidth - 1.0;
			Pos.y = -(Pos.y * 2.0 / WindowHeight - 1.0);
			return Pos;
		}
		
		void main()
		{
			gl_Position = vec4(transform_window_2_opengl(Position), 0.0, 1.0);
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

//GLuint make_text_shader()
//{
//	const char *VertexShader = R"(
//		#version 330 core
//	
//		layout(location = 0) in vec2 Position;
//		layout(location = 1) in vec2 vTex;
//		layout(location = 2) in vec4 vColor;
//	
//		out vec2 fTex;
//		out vec4 fColor;
//	
//		void main()
//		{
//			gl_Position = vec4(Position, 0.0, 1.0);
//			fTex = vTex;
//			fColor = vColor;
//		}
//	)";
//
//	const char *FragmentShader = R"(
//		#version 330 core
//		
//		in vec2 fTex;
//		in vec4 fColor;
//		
//		out vec4 Color;
//		
//		uniform sampler2D s;
//		
//		void main()
//		{
//			Color = vec4(fColor.rgb, fColor.a * texture(s, fTex).r);
////			Color = vec4(1.0, 1.0, 1.0, texture(s, fTex).r);
////			Color = vec4(texture(s, fTex).r, 1.0, 1.0, 1.0);
//		}
//	)";
//
//	return make_shader(VertexShader, FragmentShader);
//}

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

		vec2 transform_window_2_opengl(vec2 Pos)
		{
			Pos.x = Pos.x * 2.0 / WindowWidth - 1.0;
			Pos.y = -(Pos.y * 2.0 / WindowHeight - 1.0);
			return Pos;
		}
	
		void main()
		{
			gl_Position = vec4(transform_window_2_opengl(Position), 0.0, 1.0);
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

//GLuint make_texture_shader()
//{
//	const char *VertexShader = R"(
//		#version 330 core
//		
//		layout(location = 0) in vec3 Position;
//		layout(location = 1) in vec2 vTex;
//		
//		out vec2 fTex;
//		
//		void main()
//		{
//			gl_Position = vec4(Position, 1.0);
//			fTex = vTex;
//		}
//	)";
//	const char *FragmentShader = R"(
//		#version 330 core
//		
//		in vec2 fTex;
//		out vec4 Color;
//		
//		uniform sampler2D s;
//		
//		void main()
//		{
//			Color = texture(s, fTex);
//		}
//	)";
//	return make_shader(VertexShader, FragmentShader);
//}

GLuint make_texture_shader_with_transform()
{
	const char *VertexShader = R"(
		#version 330 core
		
		layout(location = 0) in vec2 Position;
		layout(location = 1) in vec2 vTex;
		
		out vec2 fTex;

		uniform float WindowWidth;
		uniform float WindowHeight;

		vec2 transform_window_2_opengl(vec2 Pos)
		{
			Pos.x = Pos.x * 2.0 / WindowWidth - 1.0;
			Pos.y = -(Pos.y * 2.0 / WindowHeight - 1.0);
			return Pos;
		}
		
		void main()
		{
			gl_Position = vec4(transform_window_2_opengl(Position), 0.0, 1.0);
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

//openglXYWH transform_window_to_opengl(windowXYWH WindowCoord, windowWH WindowSize)
//{
//	float PixelWidth = 2.0f / WindowSize.W;
//	float PixelHeight = 2.0f / WindowSize.H;
//
//	openglXYWH openglCoord;
//	openglCoord.X = WindowCoord.X * PixelWidth - 1.0f;
//	openglCoord.Y = 1.0f - WindowCoord.Y * PixelHeight;
//	openglCoord.W = WindowCoord.W * PixelWidth;
//	openglCoord.H = WindowCoord.H * PixelHeight;
//
//	return openglCoord;
//}

//void make_quad(array<float> *Vertices, openglXYWH PosAndSize, color Color)
//{
//	float X0 = PosAndSize.X;
//	float X1 = PosAndSize.X + PosAndSize.W;
//	float Y0 = PosAndSize.Y;
//	float Y1 = PosAndSize.Y - PosAndSize.H;
//
//	float QuadVertices[] = {
//		/* upper-left*/  X0, Y0, Color.R, Color.G, Color.B, Color.A,
//		/* upper-right*/ X1, Y0, Color.R, Color.G, Color.B, Color.A,
//		/* lower-right*/ X1, Y1, Color.R, Color.G, Color.B, Color.A,
//
//		/* lower-right*/ X1, Y1, Color.R, Color.G, Color.B, Color.A,
//		/* lower-left*/  X0, Y1, Color.R, Color.G, Color.B, Color.A,
//		/* upper-left*/  X0, Y0, Color.R, Color.G, Color.B, Color.A,
//	};
//	ArrayAdd(Vertices, QuadVertices, COUNT(QuadVertices));
//}
//
//// Transformation of coordinates/dimensions to OpenGL coordinates/dimensions is done here, not in the shader.
//void make_quad(array<float> *Vertices, int X, int Y, int W, int H, color Color, windowWH WindowSize)
//{
//	float PixelWidth = 2.0f / WindowSize.W;
//	float PixelHeight = 2.0f / WindowSize.H;
//
//	float _X = X * PixelWidth - 1.0f;
//	float _Y = 1.0f - Y * PixelHeight;
//	float _W = W * PixelWidth;
//	float _H = H * PixelHeight;
//
//	float X0 = _X;
//	float X1 = _X + _W;
//	float Y0 = _Y;
//	float Y1 = _Y - _H;
//
//	float QuadVertices[] = {
//		/* upper-left*/  X0, Y0, Color.R, Color.G, Color.B, Color.A,
//		/* upper-right*/ X1, Y0, Color.R, Color.G, Color.B, Color.A,
//		/* lower-right*/ X1, Y1, Color.R, Color.G, Color.B, Color.A,
//
//		/* lower-right*/ X1, Y1, Color.R, Color.G, Color.B, Color.A,
//		/* lower-left*/  X0, Y1, Color.R, Color.G, Color.B, Color.A,
//		/* upper-left*/  X0, Y0, Color.R, Color.G, Color.B, Color.A,
//	};
//	ArrayAdd(Vertices, QuadVertices, COUNT(QuadVertices));
//}

//void draw_2d_with_color(array<float> *Vertices)
//{
//	GLuint VAO;
//	glGenVertexArrays(1, &VAO);
//	glBindVertexArray(VAO);
//
//	GLuint VBO;
//	glGenBuffers(1, &VBO);
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//
//	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0); // pos
//	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (2 * sizeof(float))); // color
//	glEnableVertexAttribArray(0);
//	glEnableVertexAttribArray(1);
//
////	glBindVertexArray(QuadVAO);
////	glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
//	glBufferData(GL_ARRAY_BUFFER, Vertices->Count * sizeof(float), Vertices->Data, GL_STREAM_DRAW);
//
//	assert((Vertices->Count % 6) == 0);
//	int NumVertices = Vertices->Count / 6;
//	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
//
//	glDeleteBuffers(1, &VBO);
//	glDeleteVertexArrays(1, &VAO);
//}

void draw_quad(int X, int Y, int W, int H, color Color)
{
	float X0 = X;
	float X1 = X + W;
	float Y0 = Y;
	float Y1 = Y + H;

	float QuadVertices[] = {
		/* upper-left*/  X0, Y0, Color.R, Color.G, Color.B, Color.A,
		/* upper-right*/ X1, Y0, Color.R, Color.G, Color.B, Color.A,
		/* lower-right*/ X1, Y1, Color.R, Color.G, Color.B, Color.A,

		/* lower-right*/ X1, Y1, Color.R, Color.G, Color.B, Color.A,
		/* lower-left*/  X0, Y1, Color.R, Color.G, Color.B, Color.A,
		/* upper-left*/  X0, Y0, Color.R, Color.G, Color.B, Color.A,
	};

	glUseProgram(Shaders.ColorShader);
	pass_to_shader(Shaders.ColorShader, "WindowWidth", (float)Shaders.WindowWidth);
	pass_to_shader(Shaders.ColorShader, "WindowHeight", (float)Shaders.WindowHeight);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0); // pos
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (2 * sizeof(float))); // color
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STREAM_DRAW);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void draw_quad(int X, int Y, int W, int H, GLuint Texture, bool Flip)
{
	float X0 = X;
	float X1 = X + W;
	float Y0 = Y;
	float Y1 = Y + H;

	float TX0 = 0.0f; // left
	float TX1 = 1.0f; // right
	float TY0 = 0.0f; // upper
	float TY1 = 1.0f; // lower

	if(Flip)
	{
		TY0 = 1.0f;
		TY1 = 0.0f;
	}

	float QuadVertices[] = {
		/* upper-left*/  X0, Y0, TX0, TY0,
		/* upper-right*/ X1, Y0, TX1, TY0,
		/* lower-right*/ X1, Y1, TX1, TY1,

		/* lower-right*/ X1, Y1, TX1, TY1,
		/* lower-left*/  X0, Y1, TX0, TY1,
		/* upper-left*/  X0, Y0, TX0, TY0,
	};

	glBindTexture(GL_TEXTURE_2D, Texture);
	glUseProgram(Shaders.TextureShader);
	pass_to_shader(Shaders.TextureShader, "WindowWidth", (float)Shaders.WindowWidth);
	pass_to_shader(Shaders.TextureShader, "WindowHeight", (float)Shaders.WindowHeight);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) 0); // position
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *) (2 * sizeof(float))); // texture
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STREAM_DRAW);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

//GLuint CreateShader(const char *VertexShaderFile, const char *FragmentShaderFile)
//{
//	int success;
//	char info_log[512];
//
//	char *vertexShaderText;
//	if(!ReadTextFile(VertexShaderFile, &vertexShaderText)){
//		fprintf(stderr, "Error: couldnt read shader file: %s\n", VertexShaderFile);
//		return 0; //@
//	}
//		
//	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//	glShaderSource(vertexShader, 1, &vertexShaderText, NULL);
//	glCompileShader(vertexShader);
//
//	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
//	if (!success) {
//		glGetShaderInfoLog(vertexShader, sizeof(info_log), NULL, info_log);
//		printf("Shader error: %s: %s\n", VertexShaderFile, info_log);
//	}
//
//	char *fragmentShaderText;
//	if(!ReadTextFile(FragmentShaderFile, &fragmentShaderText)){
//		fprintf(stderr, "Error: couldnt read shader file: %s\n", fragmentShaderText);
//		return 0; //@
//	}
//		
//	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(fragmentShader, 1, &fragmentShaderText, NULL);
//	glCompileShader(fragmentShader);
//
//	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
//	if (!success) {
//		glGetShaderInfoLog(fragmentShader, sizeof(info_log), NULL, info_log);
//		printf("Shader error: %s: %s\n", FragmentShaderFile, info_log);
//	}
//
//	GLuint shaderProgram = glCreateProgram();
//	glAttachShader(shaderProgram, vertexShader);
//	glAttachShader(shaderProgram, fragmentShader);
//	glLinkProgram(shaderProgram);
//
//	//@ delete shaders?
//	free(vertexShaderText);
//	free(fragmentShaderText);
//
//	return shaderProgram;
//}

image *make_image(unsigned char *Data, int Width, int Height, int NumChannels)
{
	GLuint Tex;
	glGenTextures(1, &Tex);
	glBindTexture(GL_TEXTURE_2D, Tex);

	// What to do when texture coordinates are outside of the texture:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// What to do when the texture is minified/magnified:
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	assert(NumChannels == 1 || NumChannels == 3);
	if(NumChannels == 1)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RED, GL_UNSIGNED_BYTE, Data);
	}
	else if(NumChannels == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Data);
	}
//	glGenerateMipmap(GL_TEXTURE_2D);

	image *Image = (image *)malloc(sizeof(image));
	Image->Tex = Tex;
	Image->W = Width;
	Image->H = Height;
	Image->NumChannels = NumChannels;

	return Image;
}

#include "stb_image.h"
image *make_image(const char *FilePath)
{
	int Width, Height, NumChannels;
	stbi_set_flip_vertically_on_load(1);
	unsigned char *Data = stbi_load(FilePath, &Width, &Height, &NumChannels, 0);
	if(!Data)
	{
		fprintf(stderr, "error: make_image(): stbi_load(): %s\n", FilePath);
		return NULL;
	}
	else
	{
		printf("make_image(): width: %d, height: %d, num channels: %d, file: %s\n",
			Width, Height, NumChannels, FilePath);
	}

	image *Image = make_image(Data, Width, Height, NumChannels);

	stbi_image_free(Data);

	return Image;
}




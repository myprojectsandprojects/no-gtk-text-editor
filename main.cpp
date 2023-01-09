#include <stdio.h>
#include <GLFW/glfw3.h>

int main()
{
	if(glfwInit() == GLFW_FALSE)
	{
		printf("error: glfwInit()\n");
		return 1;
	}
	printf("success!\n");
	return 0;
}
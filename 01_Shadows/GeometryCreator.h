#pragma once

#include "gVertexBuffer.h"

namespace GeomCreator
{
	void CreateBoxGeom(gVertexBuffer &buffer_Box)
	{
		buffer_Box.AddAttribute(0, 3);
		buffer_Box.AddAttribute(1, 3);
		buffer_Box.AddAttribute(2, 2);

		buffer_Box.AddData(0, -1.0f, -1.0f, -1.0f);
		buffer_Box.AddData(0, 1.0f, -1.0f, -1.0f);
		buffer_Box.AddData(0, 1.0f, 1.0f, -1.0f);
		buffer_Box.AddData(0, 1.0f, 1.0f, -1.0f);
		buffer_Box.AddData(0, -1.0f, 1.0f, -1.0f);
		buffer_Box.AddData(0, -1.0f, -1.0f, -1.0f);

		buffer_Box.AddData(0, -1.0f, -1.0f, 1.0f);
		buffer_Box.AddData(0, 1.0f, -1.0f, 1.0f);
		buffer_Box.AddData(0, 1.0f, 1.0f, 1.0f);
		buffer_Box.AddData(0, 1.0f, 1.0f, 1.0f);
		buffer_Box.AddData(0, -1.0f, 1.0f, 1.0f);
		buffer_Box.AddData(0, -1.0f, -1.0f, 1.0f);

		buffer_Box.AddData(0, -1.0f, 1.0f, 1.0f);
		buffer_Box.AddData(0, -1.0f, 1.0f, -1.0f);
		buffer_Box.AddData(0, -1.0f, -1.0f, -1.0f);
		buffer_Box.AddData(0, -1.0f, -1.0f, -1.0f);
		buffer_Box.AddData(0, -1.0f, -1.0f, 1.0f);
		buffer_Box.AddData(0, -1.0f, 1.0f, 1.0f);

		buffer_Box.AddData(0, 1.0f, 1.0f, 1.0f);
		buffer_Box.AddData(0, 1.0f, 1.0f, -1.0f);
		buffer_Box.AddData(0, 1.0f, -1.0f, -1.0f);
		buffer_Box.AddData(0, 1.0f, -1.0f, -1.0f);
		buffer_Box.AddData(0, 1.0f, -1.0f, 1.0f);
		buffer_Box.AddData(0, 1.0f, 1.0f, 1.0f);

		buffer_Box.AddData(0, -1.0f, -1.0f, -1.0f);
		buffer_Box.AddData(0, 1.0f, -1.0f, -1.0f);
		buffer_Box.AddData(0, 1.0f, -1.0f, 1.0f);
		buffer_Box.AddData(0, 1.0f, -1.0f, 1.0f);
		buffer_Box.AddData(0, -1.0f, -1.0f, 1.0f);
		buffer_Box.AddData(0, -1.0f, -1.0f, -1.0f);

		buffer_Box.AddData(0, -1.0f, 1.0f, -1.0f);
		buffer_Box.AddData(0, 1.0f, 1.0f, -1.0f);
		buffer_Box.AddData(0, 1.0f, 1.0f, 1.0f);
		buffer_Box.AddData(0, 1.0f, 1.0f, 1.0f);
		buffer_Box.AddData(0, -1.0f, 1.0f, 1.0f);
		buffer_Box.AddData(0, -1.0f, 1.0f, -1.0f);

		buffer_Box.AddData(1, 0, 0, -1.0f);
		buffer_Box.AddData(1, 0, 0, -1.0f);
		buffer_Box.AddData(1, 0, 0, -1.0f);
		buffer_Box.AddData(1, 0, 0, -1.0f);
		buffer_Box.AddData(1, 0, 0, -1.0f);
		buffer_Box.AddData(1, 0, 0, -1.0f);

		buffer_Box.AddData(1, 0, 0, 1.0f);
		buffer_Box.AddData(1, 0, 0, 1.0f);
		buffer_Box.AddData(1, 0, 0, 1.0f);
		buffer_Box.AddData(1, 0, 0, 1.0f);
		buffer_Box.AddData(1, 0, 0, 1.0f);
		buffer_Box.AddData(1, 0, 0, 1.0f);

		buffer_Box.AddData(1, -1.0f, 0, 0);
		buffer_Box.AddData(1, -1.0f, 0, 0);
		buffer_Box.AddData(1, -1.0f, 0, 0);
		buffer_Box.AddData(1, -1.0f, 0, 0);
		buffer_Box.AddData(1, -1.0f, 0, 0);
		buffer_Box.AddData(1, -1.0f, 0, 0);

		buffer_Box.AddData(1, 1.0f, 0, 0);
		buffer_Box.AddData(1, 1.0f, 0, 0);
		buffer_Box.AddData(1, 1.0f, 0, 0);
		buffer_Box.AddData(1, 1.0f, 0, 0);
		buffer_Box.AddData(1, 1.0f, 0, 0);
		buffer_Box.AddData(1, 1.0f, 0, 0);

		buffer_Box.AddData(1, 0, -1.0f, 0);
		buffer_Box.AddData(1, 0, -1.0f, 0);
		buffer_Box.AddData(1, 0, -1.0f, 0);
		buffer_Box.AddData(1, 0, -1.0f, 0);
		buffer_Box.AddData(1, 0, -1.0f, 0);
		buffer_Box.AddData(1, 0, -1.0f, 0);

		buffer_Box.AddData(1, 0, 1.0f, 0);
		buffer_Box.AddData(1, 0, 1.0f, 0);
		buffer_Box.AddData(1, 0, 1.0f, 0);
		buffer_Box.AddData(1, 0, 1.0f, 0);
		buffer_Box.AddData(1, 0, 1.0f, 0);
		buffer_Box.AddData(1, 0, 1.0f, 0);

		buffer_Box.AddData(2, 0.0f, 0.0f);
		buffer_Box.AddData(2, 1.0f, 0.0f);
		buffer_Box.AddData(2, 1.0f, 1.0f);
		buffer_Box.AddData(2, 1.0f, 1.0f);
		buffer_Box.AddData(2, 0.0f, 1.0f);
		buffer_Box.AddData(2, 0.0f, 0.0f);

		buffer_Box.AddData(2, 0.0f, 0.0f);
		buffer_Box.AddData(2, 1.0f, 0.0f);
		buffer_Box.AddData(2, 1.0f, 1.0f);
		buffer_Box.AddData(2, 1.0f, 1.0f);
		buffer_Box.AddData(2, 0.0f, 1.0f);
		buffer_Box.AddData(2, 0.0f, 0.0f);

		buffer_Box.AddData(2, 1.0f, 0.0f);
		buffer_Box.AddData(2, 1.0f, 1.0f);
		buffer_Box.AddData(2, 0.0f, 1.0f);
		buffer_Box.AddData(2, 0.0f, 1.0f);
		buffer_Box.AddData(2, 0.0f, 0.0f);
		buffer_Box.AddData(2, 1.0f, 0.0f);

		buffer_Box.AddData(2, 1.0f, 0.0f);
		buffer_Box.AddData(2, 1.0f, 1.0f);
		buffer_Box.AddData(2, 0.0f, 1.0f);
		buffer_Box.AddData(2, 0.0f, 1.0f);
		buffer_Box.AddData(2, 0.0f, 0.0f);
		buffer_Box.AddData(2, 1.0f, 0.0f);

		buffer_Box.AddData(2, 0.0f, 1.0f);
		buffer_Box.AddData(2, 1.0f, 1.0f);
		buffer_Box.AddData(2, 1.0f, 0.0f);
		buffer_Box.AddData(2, 1.0f, 0.0f);
		buffer_Box.AddData(2, 0.0f, 0.0f);
		buffer_Box.AddData(2, 0.0f, 1.0f);

		buffer_Box.AddData(2, 0.0f, 1.0f);
		buffer_Box.AddData(2, 1.0f, 1.0f);
		buffer_Box.AddData(2, 1.0f, 0.0f);
		buffer_Box.AddData(2, 1.0f, 0.0f);
		buffer_Box.AddData(2, 0.0f, 0.0f);
		buffer_Box.AddData(2, 0.0f, 1.0f);
	}
	void CreateQuadGeom(gVertexBuffer &buffer)
	{
		buffer.AddAttribute(0, 3);
		buffer.AddData(0, glm::vec3(-1.0f, 1.0f, 0.0f));
		buffer.AddData(0, glm::vec3(-1.0f, -1.0f, 0.0f));
		buffer.AddData(0, glm::vec3(1.0f, 1.0f, 0.0f));
		buffer.AddData(0, glm::vec3(1.0f, -1.0f, 0.0f));

		buffer.AddAttribute(1, 3);
		buffer.AddData(1, glm::vec3(0, 0, 1));
		buffer.AddData(1, glm::vec3(0, 0, 1));
		buffer.AddData(1, glm::vec3(0, 0, 1));
		buffer.AddData(1, glm::vec3(0, 0, 1));

		buffer.AddAttribute(2, 2);
		buffer.AddData(2, glm::vec2(0, 0));
		buffer.AddData(2, glm::vec2(0, 1));
		buffer.AddData(2, glm::vec2(1, 0));
		buffer.AddData(2, glm::vec2(1, 1));

		buffer.AddIndex(0, 1, 2);
		buffer.AddIndex(2, 1, 3);
	}
}
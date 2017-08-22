#pragma once

#include "gVertexBuffer.h"
#include <oglwrap\oglwrap.h>
#include "LocationDefinitions.h"

namespace Geom
{

	void CreateBoxGeom(gVertexBuffer &buffer_Box)
	{
		enum CUBESIDE {
			BOTTOM = 0,
			SIDE = 1,
			TOP = 2,
			OTHER = 3,
		};
		buffer_Box.AddAttribute(LOCATION::POSITION, 3);
		buffer_Box.AddAttribute(LOCATION::NORMAL, 3);
		buffer_Box.AddAttribute(LOCATION::UV, 2);
		std::vector<int> cubeSideInfo;
		cubeSideInfo.reserve(36);
		auto loadCubeSide = [&cubeSideInfo](CUBESIDE side){
			for(int i = 0 ; i < 6; i++)
				cubeSideInfo.push_back((int)side);
		};
		
		//Front side
		buffer_Box.AddData(0, -1, -1, 1);	buffer_Box.AddData(2, 0, 1);
		buffer_Box.AddData(0, 1, -1, 1);	buffer_Box.AddData(2, 1, 1);
		buffer_Box.AddData(0, 1, 1, 1);		buffer_Box.AddData(2, 1, 0);
		buffer_Box.AddData(0, 1, 1, 1);		buffer_Box.AddData(2, 1, 0);
		buffer_Box.AddData(0, -1, 1, 1);	buffer_Box.AddData(2, 0, 0);
		buffer_Box.AddData(0, -1, -1, 1);	buffer_Box.AddData(2, 0, 1);
		loadCubeSide(CUBESIDE::SIDE);

		//Back side
		buffer_Box.AddData(0, -1, -1, -1);	buffer_Box.AddData(2, 0, 1);
		buffer_Box.AddData(0, -1, 1, -1);	buffer_Box.AddData(2, 0, 0);
		buffer_Box.AddData(0, 1, -1, -1);	buffer_Box.AddData(2, 1, 1);
		buffer_Box.AddData(0, 1, 1, -1);	buffer_Box.AddData(2, 1, 0);
		buffer_Box.AddData(0, 1, -1, -1);	buffer_Box.AddData(2, 1, 1);
		buffer_Box.AddData(0, -1, 1, -1);	buffer_Box.AddData(2, 0, 0);
		loadCubeSide(CUBESIDE::SIDE);

		//Left side
		buffer_Box.AddData(0, -1, -1, 1);	buffer_Box.AddData(2, 0, 1);
		buffer_Box.AddData(0, -1, 1, 1);	buffer_Box.AddData(2, 0, 0);
		buffer_Box.AddData(0, -1, 1, -1);	buffer_Box.AddData(2, 1, 0);
		buffer_Box.AddData(0, -1, -1, -1);	buffer_Box.AddData(2, 1, 1);
		buffer_Box.AddData(0, -1, -1, 1);	buffer_Box.AddData(2, 0, 1);
		buffer_Box.AddData(0, -1, 1, -1);	buffer_Box.AddData(2, 1, 0);
		loadCubeSide(CUBESIDE::SIDE);

		//Right Side
		buffer_Box.AddData(0, 1, 1, 1);		buffer_Box.AddData(2, 0, 0);
		buffer_Box.AddData(0, 1, -1, 1);	buffer_Box.AddData(2, 0, 1);
		buffer_Box.AddData(0, 1, 1, -1);	buffer_Box.AddData(2, 1, 0);
		buffer_Box.AddData(0, 1, -1, -1);	buffer_Box.AddData(2, 1, 1);
		buffer_Box.AddData(0, 1, 1, -1);	buffer_Box.AddData(2, 1, 0);
		buffer_Box.AddData(0, 1, -1, 1);	buffer_Box.AddData(2, 0, 1);
		loadCubeSide(CUBESIDE::SIDE);

		//Bottom
		buffer_Box.AddData(0, -1, -1, -1);	buffer_Box.AddData(2, 0, 1);
		buffer_Box.AddData(0, 1, -1, -1);	buffer_Box.AddData(2, 1, 1);
		buffer_Box.AddData(0, 1, -1, 1);	buffer_Box.AddData(2, 1, 0);
		buffer_Box.AddData(0, 1, -1, 1);	buffer_Box.AddData(2, 1, 0);
		buffer_Box.AddData(0, -1, -1, 1);	buffer_Box.AddData(2, 0, 0);
		buffer_Box.AddData(0, -1, -1, -1);	buffer_Box.AddData(2, 0, 1);
		loadCubeSide(CUBESIDE::BOTTOM);

		//Top
		buffer_Box.AddData(0, -1, 1, -1);	buffer_Box.AddData(2, 0, 1);
		buffer_Box.AddData(0, 1, 1, 1);		buffer_Box.AddData(2, 1, 0);
		buffer_Box.AddData(0, 1, 1, -1);	buffer_Box.AddData(2, 1, 1);
		buffer_Box.AddData(0, -1, 1, 1);	buffer_Box.AddData(2, 0, 0);
		buffer_Box.AddData(0, 1, 1, 1);		buffer_Box.AddData(2, 1, 0);
		buffer_Box.AddData(0, -1, 1, -1);	buffer_Box.AddData(2, 0, 1);
		loadCubeSide(CUBESIDE::TOP);

		buffer_Box.AddData(1, 0, 0, -1);
		buffer_Box.AddData(1, 0, 0, -1);
		buffer_Box.AddData(1, 0, 0, -1);
		buffer_Box.AddData(1, 0, 0, -1);
		buffer_Box.AddData(1, 0, 0, -1);
		buffer_Box.AddData(1, 0, 0, -1);

		buffer_Box.AddData(1, 0, 0, 1);
		buffer_Box.AddData(1, 0, 0, 1);
		buffer_Box.AddData(1, 0, 0, 1);
		buffer_Box.AddData(1, 0, 0, 1);
		buffer_Box.AddData(1, 0, 0, 1);
		buffer_Box.AddData(1, 0, 0, 1);

		buffer_Box.AddData(1, -1, 0, 0);
		buffer_Box.AddData(1, -1, 0, 0);
		buffer_Box.AddData(1, -1, 0, 0);
		buffer_Box.AddData(1, -1, 0, 0);
		buffer_Box.AddData(1, -1, 0, 0);
		buffer_Box.AddData(1, -1, 0, 0);

		buffer_Box.AddData(1, 1, 0, 0);
		buffer_Box.AddData(1, 1, 0, 0);
		buffer_Box.AddData(1, 1, 0, 0);
		buffer_Box.AddData(1, 1, 0, 0);
		buffer_Box.AddData(1, 1, 0, 0);
		buffer_Box.AddData(1, 1, 0, 0);

		buffer_Box.AddData(1, 0, -1, 0);
		buffer_Box.AddData(1, 0, -1, 0);
		buffer_Box.AddData(1, 0, -1, 0);
		buffer_Box.AddData(1, 0, -1, 0);
		buffer_Box.AddData(1, 0, -1, 0);
		buffer_Box.AddData(1, 0, -1, 0);

		buffer_Box.AddData(1, 0, 1, 0);
		buffer_Box.AddData(1, 0, 1, 0);
		buffer_Box.AddData(1, 0, 1, 0);
		buffer_Box.AddData(1, 0, 1, 0);
		buffer_Box.AddData(1, 0, 1, 0);
		buffer_Box.AddData(1, 0, 1, 0);

		buffer_Box.InitBuffers ();

		GLuint vboID;
		glGenBuffers(1, &vboID);
		gl::ArrayBuffer VBO(vboID);
		buffer_Box.setBufferOwner(vboID);

		buffer_Box.On();
		{
			auto bind = gl::MakeTemporaryBind (VBO);
			VBO.data(cubeSideInfo, gl::kStaticDraw);

			gl::VertexAttrib attrib(LOCATION::CUBE_SIDE);
			attrib.enable();
			attrib.ipointer (1);
		}
		buffer_Box.Off();
	}
	void CreateQuadGeom(gVertexBuffer &buffer)
	{
		buffer.AddAttribute(LOCATION::POSITION, 3);
		buffer.AddData(0, glm::vec3(-1.0f, 1.0f, 0.0f));
		buffer.AddData(0, glm::vec3(-1.0f, -1.0f, 0.0f));
		buffer.AddData(0, glm::vec3(1.0f, 1.0f, 0.0f));
		buffer.AddData(0, glm::vec3(1.0f, -1.0f, 0.0f));

		buffer.AddAttribute(LOCATION::NORMAL, 3);
		buffer.AddData(1, glm::vec3(0, 0, 1));
		buffer.AddData(1, glm::vec3(0, 0, 1));
		buffer.AddData(1, glm::vec3(0, 0, 1));
		buffer.AddData(1, glm::vec3(0, 0, 1));

		buffer.AddAttribute(LOCATION::UV, 2);
		buffer.AddData(2, glm::vec2(0, 0));
		buffer.AddData(2, glm::vec2(0, 1));
		buffer.AddData(2, glm::vec2(1, 0));
		buffer.AddData(2, glm::vec2(1, 1));

		buffer.AddIndex(0, 1, 2);
		buffer.AddIndex(2, 1, 3);

		buffer.InitBuffers ();
	}

	void CreateCoordAxes(gVertexBuffer &buffer)
	{
		buffer.AddAttribute(LOCATION::POSITION, 3);
		buffer.AddAttribute(LOCATION::COLOR, 3);
		glm::vec3 XColor = glm::vec3(1,0,0);
		glm::vec3 YColor = glm::vec3(0,1,0);
		glm::vec3 ZColor = glm::vec3(0,0,1);
		float arrowSize = 0.15;
		//X axis
		buffer.AddData(LOCATION::POSITION, glm::vec3(0, 0, 0)); buffer.AddData (LOCATION::COLOR, XColor);
		buffer.AddData(LOCATION::POSITION, glm::vec3(1, 0, 0)); buffer.AddData (LOCATION::COLOR, XColor);
		//X arrow
		buffer.AddData(LOCATION::POSITION, glm::vec3(1, 0, 0)); buffer.AddData(LOCATION::COLOR, XColor);
		buffer.AddData(LOCATION::POSITION, glm::vec3(1 - arrowSize, arrowSize, 0)); buffer.AddData(LOCATION::COLOR, XColor);
		buffer.AddData(LOCATION::POSITION, glm::vec3(1, 0, 0)); buffer.AddData(LOCATION::COLOR, XColor);
		buffer.AddData(LOCATION::POSITION, glm::vec3(1 - arrowSize, -arrowSize, 0)); buffer.AddData(LOCATION::COLOR, XColor);

		//X axis
		buffer.AddData(LOCATION::POSITION, glm::vec3(0, 0, 0)); buffer.AddData (LOCATION::COLOR, YColor);
		buffer.AddData(LOCATION::POSITION, glm::vec3(0, 1, 0)); buffer.AddData (LOCATION::COLOR, YColor);
		//X arrow
		buffer.AddData(LOCATION::POSITION, glm::vec3(0, 1, 0)); buffer.AddData(LOCATION::COLOR, YColor);
		buffer.AddData(LOCATION::POSITION, glm::vec3(-arrowSize, 1 - arrowSize, 0)); buffer.AddData(LOCATION::COLOR, YColor);
		buffer.AddData(LOCATION::POSITION, glm::vec3(0, 1, 0)); buffer.AddData(LOCATION::COLOR, YColor);
		buffer.AddData(LOCATION::POSITION, glm::vec3(arrowSize, 1 - arrowSize, 0)); buffer.AddData(LOCATION::COLOR, YColor);

		//Z Axis
		buffer.AddData(LOCATION::POSITION, glm::vec3(0, 0, 0)); buffer.AddData (LOCATION::COLOR, glm::vec3(0, 0, 1));
		buffer.AddData(LOCATION::POSITION, glm::vec3(0, 0, -1)); buffer.AddData (LOCATION::COLOR, glm::vec3(0, 0, 1));
		//Z Arrow
		buffer.AddData(LOCATION::POSITION, glm::vec3(0, 0, -1)); buffer.AddData(LOCATION::COLOR, glm::vec3(0, 0, 1));
		buffer.AddData(LOCATION::POSITION, glm::vec3(arrowSize, 0, -1 + arrowSize)); buffer.AddData(LOCATION::COLOR, glm::vec3(0, 0, 1));
		buffer.AddData(LOCATION::POSITION, glm::vec3(0, 0, -1)); buffer.AddData(LOCATION::COLOR, glm::vec3(0, 0, 1));
		buffer.AddData(LOCATION::POSITION, glm::vec3(-arrowSize, 0, -1 + arrowSize)); buffer.AddData(LOCATION::COLOR, glm::vec3(0, 0, 1));
	

		buffer.InitBuffers();
	}
}
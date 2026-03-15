#include "../include/VertexArrayObject.h"

namespace kas::gui
{
	//-----------------
	
	VertexArrayObject::VertexArrayObject() :
		m_id{}
	{
		glGenVertexArrays(1, &m_id);
		bind();
	}
	
	//-----------------
	
	VertexArrayObject::~VertexArrayObject()
	{
		glDeleteVertexArrays(1, &m_id);
	}

	//-----------------

	void VertexArrayObject::bind() const
	{
		glBindVertexArray(m_id);
	}

	//-----------------

	void VertexArrayObject::unbind() const
	{
		glBindVertexArray(0);
	}

	//-----------------
}
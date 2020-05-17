#include <heatsink/traits/name.hpp>

#include <cassert>

#include <heatsink/error/exception.hpp>

namespace heatsink::gl {
	GLuint name_traits<GL_BUFFER>::create() {
		GLuint name;
		glGenBuffers(1, &name);

		if (!name)
			throw exception("gl::name_traits", "could not allocate buffer.");
		else
			return name;
	}

	void name_traits<GL_BUFFER>::destroy(GLuint name) {
		assert(name);
		glDeleteBuffers(1, &name);
	}

	void name_traits<GL_BUFFER>::bind(GLuint name, GLenum target) {
		glBindBuffer(target, name);
	}

	GLuint name_traits<GL_FRAMEBUFFER>::create() {
		GLuint name;
		glGenFramebuffers(1, &name);

		if (!name)
			throw exception("gl::name_traits", "could not allocate framebuffer.");
		else
			return name;
	}

	void name_traits<GL_FRAMEBUFFER>::destroy(GLuint name) {
		assert(name);
		glDeleteFramebuffers(1, &name);
	}

	void name_traits<GL_FRAMEBUFFER>::bind(GLuint name, GLenum target) {
		glBindFramebuffer(target, name);
	}

	GLuint name_traits<GL_PROGRAM_PIPELINE>::create() {
		GLuint name;
		glGenProgramPipelines(1, &name);

		if (!name)
			throw exception("gl::name_traits", "could not allocate program pipeline.");
		else
			return name;
	}

	void name_traits<GL_PROGRAM_PIPELINE>::destroy(GLuint name) {
		assert(name);
		glDeleteProgramPipelines(1, &name);
	}

	void name_traits<GL_PROGRAM_PIPELINE>::bind(GLuint name) {
		glBindProgramPipeline(name);
	}

	GLuint name_traits<GL_QUERY>::create() {
		GLuint name;
		glGenQueries(1, &name);

		if (!name)
			throw exception("gl::name_traits", "could not allocate query.");
		else
			return name;
	}

	void name_traits<GL_QUERY>::destroy(GLuint name) {
		assert(name);
		glDeleteQueries(1, &name);
	}

	void name_traits<GL_QUERY>::bind(GLuint name, GLenum target) {
		// FIXME: determine the best way to handle queries as objects.
		if (name)
			glBeginQuery(target, name);
		else
			glEndQuery(target);
	}

	GLuint name_traits<GL_RENDERBUFFER>::create() {
		GLuint name;
		glGenRenderbuffers(1, &name);

		if (!name)
			throw exception("gl::name_traits", "could not allocate renderbuffer.");
		else
			return name;
	}

	void name_traits<GL_RENDERBUFFER>::destroy(GLuint name) {
		assert(name);
		glDeleteRenderbuffers(1, &name);
	}

	void name_traits<GL_RENDERBUFFER>::bind(GLuint name) {
		glBindRenderbuffer(GL_RENDERBUFFER, name);
	}

	GLuint name_traits<GL_SAMPLER>::create() {
		GLuint name;
		glGenSamplers(1, &name);

		if (!name)
			throw exception("gl::name_traits", "could not allocate sampler.");
		else
			return name;
	}

	void name_traits<GL_SAMPLER>::destroy(GLuint name) {
		assert(name);
		glDeleteSamplers(1, &name);
	}

	void name_traits<GL_SAMPLER>::bind(GLuint name, std::size_t unit) {
		glBindSampler((GLuint)unit, name);
	}

	GLuint name_traits<GL_TEXTURE>::create() {
		GLuint name;
		glGenTextures(1, &name);

		if (!name)
			throw exception("gl::name_traits", "could not allocate texture.");
		else
			return name;
	}

	void name_traits<GL_TEXTURE>::destroy(GLuint name) {
		assert(name);
		glDeleteTextures(1, &name);
	}

	void name_traits<GL_TEXTURE>::bind(GLuint name, GLenum target, std::size_t unit) {
		glActiveTexture((GLenum)(GL_TEXTURE0 + unit));
		glBindTexture(target, name);
	}

	GLuint name_traits<GL_TRANSFORM_FEEDBACK>::create() {
		GLuint name;
		glGenTransformFeedbacks(1, &name);

		if (!name)
			throw exception("gl::name_traits", "could not allocate transform feedback.");
		else
			return name;
	}

	void name_traits<GL_TRANSFORM_FEEDBACK>::destroy(GLuint name) {
		assert(name);
		glDeleteTransformFeedbacks(1, &name);
	}

	void name_traits<GL_TRANSFORM_FEEDBACK>::bind(GLuint name, GLenum target) {
		glBindTransformFeedback(target, name);
	}

	GLuint name_traits<GL_VERTEX_ARRAY>::create() {
		GLuint name;
		glGenVertexArrays(1, &name);

		if (!name)
			throw exception("gl::name_traits", "could not allocate vertex array.");
		else
			return name;
	}

	void name_traits<GL_VERTEX_ARRAY>::destroy(GLuint name) {
		assert(name);
		glDeleteVertexArrays(1, &name);
	}

	void name_traits<GL_VERTEX_ARRAY>::bind(GLuint name) {
		glBindVertexArray(name);
	}
}

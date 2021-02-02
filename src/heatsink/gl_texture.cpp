#include <heatsink/gl/texture.hpp>

#include <tuple>

namespace {
	//
}

namespace heatsink::gl {
	texture texture::immutable(GLenum target, GLenum ifmt, extents es, std::size_t mips) {
		assert(!texture_traits::is_multisample(target));
		assert(es.get_length() == texture_traits::rank(target));
		assert(mips > 0);

		auto dim = es.get(1);
		// A cubemap must always be specified with six faces.
		assert((target != GL_TEXTURE_CUBE_MAP) || (dim.z == 6));
		// A cubemap array must have a multiple of six faces.
		assert((target != GL_TEXTURE_CUBE_MAP_ARRAY) || (dim.z % 6 == 0));

		// TODO: validate mip count.

		return texture(target, ifmt, es, mips);
	}

	texture texture::multisample(GLenum target, GLenum ifmt, extents es, std::size_t n, bool fix) {
		assert(texture_traits::is_multisample(target));
		assert(n > 0);

		return texture(target, ifmt, es, n, fix);
	}

	texture::texture(GLenum target)
	: object<GL_TEXTURE>(target), m_immutable{false}, m_base{}, m_format{GL_NONE} {
		assert(!texture_traits::is_multisample(target));
	}

	texture::texture(GLenum target, GLenum ifmt, extents es, std::size_t mips)
	: texture(target) {
		this->set(ifmt, es, mips);
	}

	texture::texture(const texture& t, extents offset, extents size)
	: object<GL_TXETURE>(t), m_immutable{t.m_immutable}, m_levels{t.m_levels} {
		assert(t.is_valid());
		
		auto rank = texture_traits::rank(t.get_target());
		assert(offset.get_length() == rank && size.get_length() == rank);

		// Extra size dimensions have a value of `1`, while extra offset
		// dimensions use `0` (size vs. position measurements).
		auto [bs, es] = std::make_tuple(offset.get(0), size.get(1));
		assert(!glm::any(glm::greaterThan(bs + es, t.m_extents)));

		m_base    = t.m_base + bs;
		m_extents = es;
	}

	texture::texture(GLenum target, GLenum ifmt, extents es, std::size_t mips)
	: object<GL_TEXTURE>(target), m_immutable{true}, m_base{}, m_extents{es.get(1)}, m_format{ifmt}, m_levels{mips} {
		auto t = this->get_target();

		this->bind(0);
		glTexParameteri(t, GL_TEXTURE_MAX_LEVEL, m_levels - 1);

		auto rank = texture_traits::rank(t);
		if (t == GL_TEXTURE_CUBE_MAP) {
			// Cubemap storage can be allocated using the 2D method. Note that
			// unlike setting data, storage for all 6 faces is allocated.
			rank = 2;
		}

		auto [x, y, z] = m_extents;
		switch (rank) {
			case 1: glTexStorage1D(t, m_levels, m_format, x      ); break;
			case 2: glTexStorage2D(t, m_levels, m_format, x, y   ); break;
			case 3: glTexStorage3D(t, m_levels, m_format, x, y, z); break;
		}
	}

	texture::texture(GLenum target, GLenum ifmt, extents es, std::size_t n, bool fix)
	: object<GL_TEXTURE>(target), m_immutable{true}, m_base{}, m_extents{es.get(1)}, m_format{ifmt}, m_levels{1} {
		auto t = this->get_target();
		auto [x, y, z] = m_extents;

		this->bind(0);
		switch (rank) {
			// Note that there are no 1D multisample textures.
			case 2: glTexStorage2DMultisample(t, n, m_format, x, y,    fix); break;
			case 3: glTexStorage3DMultisample(t, n, m_format, x, y, z, fix); break;
		}
	}

	void texture::set(GLenum ifmt, extents es, std::size_t mips) {
		assert(this->is_valid() && !this->is_immutable());
		assert(m_base == glm::uvec3(0));
		assert(mips > 0);

		//
	}
}

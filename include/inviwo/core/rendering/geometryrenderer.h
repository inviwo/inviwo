/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *********************************************************************************/

#ifndef IVW_GEOMETRY_RENDERER_H
#define IVW_GEOMETRY_RENDERER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/geometry/geometry.h>

namespace inviwo {

struct CanRenderGeometry;
class GeometryRendererFactory;

/** \class GeometryRenderer
 *
 * Base class for renderers capable of rendering Geometry.
 * A derived GeometryRenderer should be registered by the module.
 * The GeometryRenderFactory can be used to get a renderer
 * without knowing the type of Geometry. This is enabled by
 * implementing the abstract functions canRender and create.
 *
 * \section example Example
 * Example of how to implement a derived GeometryRenderer.
 * @code
 *    class IVW_XXX_API DerivedRenderer: public GeometryRenderer {
 *    public:
 *        DerivedRenderer(const DerivedGeometry* g): GeometryRenderer(), geomToRender_(g) {};
 *        virtual void render() { // do stuff
 *        }
 *        virtual const Geometry* getGeometry() const { return geomToRender_; }
 *    protected:
 *        virtual DerivedRenderer* create(const Geometry* geom) const { return new DerivedRenderer(static_cast<const DerivedGeometry*>(geom)); }
 *        virtual bool canRender(const Geometry* geom) const { return dynamic_cast<const DerivedGeometry*>(geom) != NULL; }
 *    private:
 *        const DerivedGeometry* geomToRender_;
 *    };
 *
 * @endcode
 *
 * @see Geometry
 * @see GeometryRendererFactory
 * @see Module
 */
class GeometryRenderer {
    friend struct CanRenderGeometry; // Access to canRender
    friend class GeometryRendererFactory; // Access to create
public:
    GeometryRenderer() {};
    virtual ~GeometryRenderer() {};

    /**
     * Render the geometry the renderer was created for.
     *
     */
    virtual void render() = 0;

    /**
     * Get the geometry to render.
     *
     * @return
     */
    virtual const Geometry* getGeometry() const = 0;

protected:

    /**
     * Return a new object of the derived class.
     *
     * @note The GeometryRender does not take ownership of the Geometry.
     * @param geom The geometry to render. This will always be of a type that canRender return true for.
     * @return A new renderer.
     */
    virtual GeometryRenderer* create(const Geometry* geom) const = 0;

    /**
     * Determine if the renderer can render geometry.
     *
     * @param geom The Geometry to render
     * @return Return true if able to render the Geometry, otherwise false.
     */
    virtual bool canRender(const Geometry* geom) const = 0;

};


} // namespace

#endif // IVW_GEOMETRY_RENDERER_H

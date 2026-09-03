/**********************************************************************
* Copyright (C) 2015 Vistinct AB
* All Rights Reserved.
*
* Unauthorized copying of this file, via any medium is strictly prohibited
* Proprietary and confidential
* No part of this software may be reproduced or transmitted in any
* form or by any means including photocopying or recording without
* written permission of the copyright owner.
*
* Primary author : Daniel Jönsson
*
**********************************************************************/

#include <inviwo/pathtracing/uniformgrid3d.h>

namespace inviwo {


UniformGrid3DBase::UniformGrid3DBase(size3_t cellDimension /*= size3_t(1)*/) : StructuredGridEntity<3>(), cellDimension_(cellDimension) {

}

UniformGrid3DBase::UniformGrid3DBase(const UniformGrid3DBase&) = default;

UniformGrid3DBase::~UniformGrid3DBase() = default;

UniformGrid3DBase& UniformGrid3DBase::operator=(const UniformGrid3DBase& that) = default;

} // namespace



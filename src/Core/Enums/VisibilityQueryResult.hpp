#pragma once

namespace nihil
{
    ///The result of a visibility query
    enum class VisibilityQueryResult
    {
        ///The object is fully outside
        Outside,
        ///The object is fully inside of the view
        Inside,
        ///The object intersects with the view boundaries
        Intersection
    };
}
# クラス図

```mermaid
classDiagram
    class Texture {
        <<interface>>
        +value(u, v, p) Color
    }

    class SolidColor {
        +value(u, v, p) Color
    }

    class CheckerTexture {
        -shared_ptr~Texture~ even
        -shared_ptr~Texture~ odd
        -float inv_scale
        +value(u, v, p) Color
    }

    class ImageTexture {
        -int width
        -int height
        -int channels
        -size_t row_stride
        -vector~uint8_t~ pixels
        +load(path) bool
        +value(u, v, p) Color
        +is_valid() bool
    }

    class Material {
        +shared_ptr~Texture~ albedo_texture
        +shared_ptr~Texture~ emission_texture
        +float metallic
        +float roughness
        +float transmission
        +float ior
        +Color absorption_coefficient
        +sample_albedo(u, v, p) Color
        +sample_emission(u, v, p) Color
    }

    class HitRecord {
        +Point3 point
        +Vec3 normal
        +float u
        +float v
        +float t
        +int material_id
        +int object_id
        +bool front_face
        +set_face_normal(ray, outward_normal)
    }

    class Object {
        <<abstract>>
        -int material_id
        -int object_id
        +hit(ray, t_min, t_max, rec) bool*
        +bounding_box() AABB*
        +set_object_id(id)
        +get_object_id() int
    }

    class Sphere {
        -Point3 center
        -float radius
        +hit(ray, t_min, t_max, rec) bool
        +bounding_box() AABB
    }

    class BVHNode {
        -AABB box
        -shared_ptr~Object~ left
        -shared_ptr~Object~ right
        -vector~shared_ptr~Object~~ leaf_objects
        -int split_axis
        -bool is_leaf
        +hit(ray, t_min, t_max, rec) bool
        +bounding_box() AABB
    }

    class EnvironmentMap {
        -int width
        -int height
        -float intensity
        -vector~Color~ pixels
        +load_hdr(path) bool
        +sample(direction) Color
        +set_intensity(value)
        +is_valid() bool
    }

    class Scene {
        -vector~shared_ptr~Object~~ objects
        -vector~Material~ materials
        -Color background
        -EnvironmentMap environment_map
        -bool has_environment
        -Vec3 sun_direction
        -float sun_intensity
        -Color sun_color
        -shared_ptr~Object~ bvh_root
        -shared_ptr~atomic~bool~~ bvh_dirty
        -shared_ptr~mutex~ bvh_mutex
        +add_object(object)
        +add_material(mat)
        +prepare_acceleration()
        +find_closest_hit(ray, t_min, t_max, rec) bool
        +sample_environment(direction) Color
    }

    class BsdfSample {
        +Vec3 wi
        +Color weight
        +float pdf
        +bool valid
        +bool is_delta
    }

    class IBSDF {
        <<interface>>
        +sample(wo, hit, material) BsdfSample
        +eval(wo, wi, hit, material) Color
        +pdf(wo, wi, hit, material) float
    }

    class PbrBsdf {
        +sample(wo, hit, material) BsdfSample
        +eval(wo, wi, hit, material) Color
        +pdf(wo, wi, hit, material) float
    }

    class Renderer {
        -int width
        -int height
        -int samples_per_pixel
        -int max_depth
        -vector~uint32_t~ pixels
        -unique_ptr~IBSDF~ bsdf
        +render(scene, camera)
        -trace_ray(ray, scene, depth)
        -evaluate_shadow_transmittance(scene, shadow_ray)
    }

    class material_optics {
        <<namespace>>
        +beer_lambert_transmittance(material, distance) Color
    }

    SolidColor ..|> Texture
    CheckerTexture ..|> Texture
    ImageTexture ..|> Texture
    Sphere --|> Object
    BVHNode --|> Object
    PbrBsdf ..|> IBSDF

    Scene --> Object : owns
    Scene --> Material : owns
    Scene --> EnvironmentMap : optional
    Scene --> BVHNode : builds/uses

    Material --> Texture : samples
    IBSDF --> Material : evaluates
    IBSDF --> HitRecord : uses
    IBSDF --> BsdfSample : returns

    Renderer --> Scene
    Renderer --> IBSDF : abstraction
    Renderer --> material_optics : attenuation
```

ポイント:

- `Scene` は `bvh_dirty`（atomic）と `bvh_mutex` で BVH の遅延再構築を制御します。
- `HitRecord` には `u/v/object_id/front_face` が入り、BSDF と透過影処理で利用されます。
- `Material` は常にテクスチャ経由で色を取得し、BRDF/BTDF の評価責務は `IBSDF` 側にあります。
#include "scene.hpp"
#include "../object/bvh.hpp"

void Scene::build_bvh() const {
    if (objects.empty()) {
        bvh_root.reset();
        bvh_dirty->store(false, std::memory_order_release);
        return;
    }

    bvh_root = std::make_shared<BVHNode>(objects, 0u, objects.size());
    bvh_dirty->store(false, std::memory_order_release);
}

void Scene::prepare_acceleration() const {
    if (!bvh_dirty->load(std::memory_order_acquire)) {
        return;
    }

    std::lock_guard<std::mutex> lock(*bvh_mutex);
    if (bvh_dirty->load(std::memory_order_relaxed)) {
        build_bvh();
    }
}

bool Scene::find_closest_hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    if (bvh_dirty->load(std::memory_order_acquire)) {
        std::lock_guard<std::mutex> lock(*bvh_mutex);
        if (bvh_dirty->load(std::memory_order_relaxed)) {
            build_bvh();
        }
    }

    std::shared_ptr<Object> local_bvh = bvh_root;
    if (local_bvh) {
        return local_bvh->hit(ray, t_min, t_max, rec);
    }
    return false;
}

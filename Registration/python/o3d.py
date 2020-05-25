# examples/Python/Basic/icp_registration.py

import open3d as o3d
import numpy as np
import copy


def draw_registration_result(source, target, transformation):
    source_temp = copy.deepcopy(source)
    target_temp = copy.deepcopy(target)
    source_temp.paint_uniform_color([1, 0.706, 0])
    target_temp.paint_uniform_color([0, 0.651, 0.929])
    source_temp.transform(transformation)
    o3d.visualization.draw_geometries([source_temp, target_temp])


if __name__ == "__main__":
    source = o3d.io.read_point_cloud("./vesseltree.pcd")
    target = o3d.io.read_point_cloud("./vesselplane.pcd")
    threshold = 5.0 # Magic number
    #threshold = 12.5
    #threshold = 0.05


    zrot = 3.0/180.0 * np.pi

    trans_init = np.asarray([[np.cos(zrot),  np.sin(zrot), 0.0, 0.0],
                             [-np.sin(zrot),  np.cos(zrot), 0.0, 0.0],
                             [0.0, 0.0, 1.0, 0.0],
                             [0.0, 0.0, 0.0, 1.0]])
    draw_registration_result(source, target, trans_init)
    print("Initial alignment")
    evaluation = o3d.registration.evaluate_registration(source, target,
                                                        threshold, trans_init)
    print(evaluation)

    print("Apply point-to-point ICP")
    crit = o3d.registration.ICPConvergenceCriteria()
    crit.max_iteration=100

    reg_p2p = o3d.registration.registration_icp(
        source, target, threshold, trans_init,
        o3d.registration.TransformationEstimationPointToPoint(),
        criteria=crit)
    print(reg_p2p)
    print("Transformation is:")
    print(reg_p2p.transformation)
    print("")
    draw_registration_result(source, target, reg_p2p.transformation)

    if 0:
      print("Apply point-to-plane ICP")
      reg_p2l = o3d.registration.registration_icp(
          source, target, threshold, trans_init,
          o3d.registration.TransformationEstimationPointToPlane())
      print(reg_p2l)
      print("Transformation is:")
      print(reg_p2l.transformation)
      print("")
      draw_registration_result(source, target, reg_p2l.transformation)

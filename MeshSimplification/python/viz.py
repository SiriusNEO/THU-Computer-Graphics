import sys
import trimesh


if __name__ == '__main__':
    args = sys.argv[1:]

    if len(args) != 1:
        print("Usage: python3 viz.py <mesh_path>")
        sys.exit(1)

    mesh = trimesh.load(args[0])
    print("Load mesh successfully from: ", args[0])
    print(mesh)
    scene = mesh.scene()

    # set visible to False because the server may not have X server
    data = scene.save_image(resolution=[256,256], visible=False)



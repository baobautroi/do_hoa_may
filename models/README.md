# Models Directory

Đặt các file .obj model của bạn vào đây.

## Cấu trúc gợi ý:

```
models/
├── tree.obj
├── tree.mtl
├── paddle.obj
├── paddle.mtl
├── player.obj
├── player.mtl
└── textures/
    ├── tree_diffuse.png
    ├── paddle_diffuse.png
    └── player_diffuse.png
```

## Tìm models miễn phí:

- Sketchfab: https://sketchfab.com/3d-models?features=downloadable&sort_by=-likeCount
- Free3D: https://free3d.com
- CGTrader: https://www.cgtrader.com/free-3d-models

## Tips:

- Chọn **Low Poly** models (< 10k triangles)
- Format: **OBJ** hoặc **FBX**
- Có texture đi kèm
- Check license (free for personal use)

## Test model:

```bash
test_model_loader.exe models/your_model.obj
```

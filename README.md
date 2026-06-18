# Quick Scatter Tool

Plugin hỗ trợ rải nhanh các Static Mesh lên bề mặt địa hình/vật thể trong Level Editor của Unreal Engine và bake toàn bộ các mesh đã rải sang dạng Hierarchical Instanced Static Mesh (HISM) để tối ưu hóa hiệu năng cảnh.

## Cấu trúc thư mục & File

```
QuickScatterTool/
├── Source/
│   └── QuickScatterTool/
│       ├── Private/
│       │   ├── QuickScatterTool.cpp    # Đăng ký module, tạo Tab Spawner và nút mở nhanh giao diện trên Play Toolbar.
│       │   └── SQuickScatterWindow.cpp  # Xây dựng giao diện Slate và xử lý logic (kéo thả mesh, thuật toán rải raycast, bake sang HISM, undo).
│       └── Public/
│           ├── QuickScatterConfig.h    # Class UObject lưu các thông số cấu hình rải (bán kính, số lượng, scale, physics).
│           ├── QuickScatterTool.h      # Định nghĩa lớp Module chính của Plugin.
│           └── SQuickScatterWindow.h   # Khai báo lớp Widget chính hiển thị giao diện của Tool.
├── QuickScatterTool.uplugin            # File mô tả thông tin cấu hình của Plugin.
└── README.md
```

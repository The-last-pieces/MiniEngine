// RGB颜色
type Color = [number, number, number]
// 三维向量
type Vec3<T = number> = [T, T, T]
// 二维向量
type Vec2<T = number> = [T, T]
// 角度值的数字
type Deg = number
// 弧度制的数字
type Rad = number
// 像素单位,需要是正整数
type PX = number
// 图片类型
type ImageType = "png" | "bmp"
// 材质类型
type MaterialType = "diffuse" | "diffuse_light" | "mirror" | "refract";
// 字典
type Dict<T> = { [key: string]: T }
// 旋转 , 如果是Vec2则先进行方位角旋转再进行仰角旋转 , 如果是Vec3则依次进行xyz的旋转
type Rotate = Vec2<Deg> | Vec3<Deg>


// 描述一个图元的信息
interface ObjectInfo<Meta> {
    "structure": Meta,
    "transform": {
        "scale": Vec3,
        "rotate": Rotate,
        "translate": Vec3
    },
    // 使用预定义的材质或者临时材质
    "material": string | { "type": MaterialType, [key: string]: any }
}

// 渲染上下文
interface RenderContext {
    "render": {
        // 渲染器类型 : 光追/光栅化
        "type": "rt" | "rs",
        // Todo 指定渲染帧数 , 多帧自动保存为视频
        // "frame": number,
        // 渲染的采样率
        "spp": PX,
        // 是否有ui
        "ui": boolean,
        // 渲染的背景色
        "background": Color
    },
    "image": {
        // 场景的名称
        "sceneName": string,
        // 图片后缀 , 图片名 = `${sceneName}/spp_{spp}_v{idx}.{fileSuffix}`
        "fileSuffix": "png" | "bmp",
        // 图片宽高, camera通过此值自动计算宽高比
        "width": PX,
        "height": PX,
    },
    "camera": {
        "eye": Vec3,
        "target": Vec3,
        // 通过eye和target算出视线向量d,通过d和-z求出旋转的方位角和仰角,依次算出旋转后的+y再绕d顺时针旋转rotate角度作为up_dir
        "rotate": Deg,
        // 摄像机在垂直方向上的视线夹角,
        "fov": Deg
    },
    "scene": {
        // 可以在这里声明在scene中使用的变量
        "vars": {
            // 一些常量 , 不能依赖其他变量
            "constants"?: Dict<number | Vec2 | Vec3>
            // 引入其他文件中的vars , 如果名称有冲突会警告
            "imports": string[]
            // 材质的声明 , 可以使用合并后的constants中的变量
            "materials"?: {
                // 漫反射材质
                "diffuse"?: Dict<{
                    // 纯色值
                    "solid"?: Color,
                    // 图片路径
                    "image"?: string
                }>,
                "diffuse_light"?: Dict<{
                    // 光照强度
                    emit: Color
                }>,
                "mirror"?: Dict<{
                    // 吸收率
                    albedo: Color
                }>,
                "refract"?: Dict<{
                    // 反射率
                    index: number
                }>
            },
        },
        // 图元 , material字段可以引用合并后的预定义材质 , 只有临时材质可以使用constants
        "objects": {
            // 球体
            "sphere": ObjectInfo<{}>[],
            // 平面
            "flat": ObjectInfo<{}>[],
        },
        // 模型
        "models": [
            {
                "objPath": string;
                "texturePath": string;
                "shaderType": "vertex" | "fragment";
            }
        ]
    }
}
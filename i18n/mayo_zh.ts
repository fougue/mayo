<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>Mayo::AppModule</name>
    <message>
        <location filename="../src/app/app_module.cpp" line="124"/>
        <source>en</source>
        <translation>英语</translation>
    </message>
    <message>
        <location filename="../src/app/app_module.cpp" line="125"/>
        <source>fr</source>
        <translation>法语</translation>
    </message>
    <message>
        <location filename="../src/app/app_module.cpp" line="126"/>
        <source>zh</source>
        <translation>中文</translation>
    </message>
</context>
<context>
    <name>Mayo::AppModuleProperties</name>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="43"/>
        <source>language</source>
        <translation>语言</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="37"/>
        <source>system</source>
        <translation>系统</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="25"/>
        <source>TopLeft</source>
        <translation>左上角</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="26"/>
        <source>TopRight</source>
        <translation>右上角</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="27"/>
        <source>BottomLeft</source>
        <translation>左下角</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="28"/>
        <source>BottomRight</source>
        <translation>右下角</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="38"/>
        <source>application</source>
        <translation>应用程序</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="39"/>
        <source>meshing</source>
        <translation>BRep 网格化</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="40"/>
        <source>graphics</source>
        <translation>图形</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="44"/>
        <source>viewCubeCorner</source>
        <translation>视图立方体位置</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="47"/>
        <source>units</source>
        <translation>单位</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="48"/>
        <source>clipPlanes</source>
        <translation>剖切平面</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="49"/>
        <source>meshDefaults</source>
        <translation>网格默认值</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="41"/>
        <source>import</source>
        <translation>导入</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="42"/>
        <source>export</source>
        <translation>导出</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="189"/>
        <source>Language used for the application. Change will take effect after application restart</source>
        <translation>应用程序使用的语言。更改将在重启应用程序后生效</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="193"/>
        <source>Action to be done after some opened document file is changed(modified) externally

Select options `{0}` or `{1}` so the application monitors changes made to opened files

When such a change is detected then the application proposes to reload(open again) the document

Select `{1}` to automatically reload documents without any user interaction</source>
        <translation>外部修改已打开的文档文件后的处理方式

选择 `{0}` 或 `{1}` 选项，使应用程序监控已打开文件的变化

检测到此类变化时，应用程序会提示重新加载（重新打开）文档

选择 `{1}` 可无需用户操作自动重新加载文档</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="202"/>
        <source>In case where multiple documents are opened, make sure the document displayed in the 3D view corresponds to what is selected in the model tree</source>
        <translation>当多个文档同时打开时，确保 3D 视图中显示的文档与模型树中所选内容一致</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="206"/>
        <source>Force usage of the fallback Qt widget to display OpenGL graphics.

When `OFF` the application will try to use OpenGL framebuffer for rendering, this allows to display overlay widgets(eg measure tools panel) with translucid background. However using OpenGL framebuffer might cause troubles for some users(eg empty 3D window) especially on macOS.

When `ON` the application will use a regular Qt widget for rendering which proved to be more supported.

This option is applicable when OpenCascade ≥ 7.6 version. Change will take effect after application restart</source>
        <translation>强制使用备用 Qt 控件显示 OpenGL 图形。

关闭时，应用程序将尝试使用 OpenGL 帧缓冲区进行渲染，这允许以半透明背景显示叠加控件（例如测量工具面板）。但对部分用户（尤其是 macOS 用户）使用 OpenGL 帧缓冲区可能导致问题（如 3D 窗口空白）。

开启时，应用程序将使用常规 Qt 控件进行渲染，兼容性更好。

此选项适用于 OpenCascade ≥ 7.6 版本。更改将在重启应用程序后生效</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="220"/>
        <source>Controls precision of the mesh to be computed from the BRep shape</source>
        <translation>控制从 BRep 形状计算网格的精度</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="223"/>
        <source>For the tessellation of faces the chordal deflection limits the distance between a curve and its tessellation</source>
        <translation>在面的三角化中，弦高误差限制曲线与其离散结果之间的距离</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="227"/>
        <source>For the tessellation of faces the angular deflection limits the angle between subsequent segments in a polyline</source>
        <translation>在面的曲面细分中，角偏差限制了折线中相邻线段之间的角度</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="231"/>
        <source>Relative computation of edge tolerance

If activated, deflection used for the polygonalisation of each edge will be `ChordalDeflection` &amp;#215; `SizeOfEdge`. The deflection used for the faces will be the maximum deflection of their edges.</source>
        <translation>边容差的相对计算

若激活，每条边多边形化所用的偏差为 `ChordalDeflection`  &amp;#215; `SizeOfEdge`。面所用的偏差为其各边偏差的最大值。</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="239"/>
        <source>3D view manipulation shortcuts configuration to mimic other common CAD applications</source>
        <translation>3D 视图操作快捷键配置，以模拟其他常用 CAD 软件的操作方式</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="242"/>
        <source>Angle increment used to turn(rotate) the 3D view around the normal of the view plane(Z axis frame reference)</source>
        <translation>围绕视图平面法线（Z 轴参考系）旋转 3D 视图时所用的角度增量</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="244"/>
        <source>Corner where 3D view cube is located</source>
        <translation>3D 视图立方体所在的角落</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="248"/>
        <source>Show or hide by default the trihedron centered at world origin. This doesn&apos;t affect 3D view of currently opened documents</source>
        <translation>默认显示或隐藏位于世界原点的三面体。此设置不影响当前已打开文档的 3D 视图</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="252"/>
        <source>Enable capping of currently clipped graphics</source>
        <translation>启用当前剖切图形的封口</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="255"/>
        <source>Enable capping hatch texture of currently clipped graphics</source>
        <translation>启用当前剖切图形封口的填充纹理</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="260"/>
        <source>Automatically expand compound shapes to assemblies. For some input models this allows 3D exploding</source>
        <translation>自动将复合形状展开为装配体。对于某些输入模型，此选项允许进行 3D 爆炸视图</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="63"/>
        <source>decimalCount</source>
        <translation>小数位数</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="64"/>
        <source>schema</source>
        <translation>模式</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="67"/>
        <source>recentFiles</source>
        <translation>最近文件</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="68"/>
        <source>lastOpenFolder</source>
        <translation>上次打开的文件夹</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="69"/>
        <source>lastSelectedFormatFilter</source>
        <translation>上次选择的格式过滤器</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="70"/>
        <source>actionOnDocumentFileChange</source>
        <translation>文档文件变更后的操作</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="71"/>
        <source>linkWithDocumentSelector</source>
        <translation>与文档选择器联动</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="72"/>
        <source>forceOpenGlFallbackWidget</source>
        <translation>强制使用 OpenGL 备用控件</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="73"/>
        <source>appUiState</source>
        <translation>应用程序界面状态</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="76"/>
        <source>meshingQuality</source>
        <translation>质量</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="77"/>
        <source>meshingChordalDeflection</source>
        <translation>弦偏差</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="78"/>
        <source>meshingAngularDeflection</source>
        <translation>角偏差</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="79"/>
        <source>meshingRelative</source>
        <translation>相对值</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="81"/>
        <source>navigationStyle</source>
        <translation>视图导航样式</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="83"/>
        <source>defaultShowOriginTrihedron</source>
        <translation>默认显示原点三面体</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="84"/>
        <source>instantZoomFactor</source>
        <translation>即时缩放倍率</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="85"/>
        <source>turnViewAngleIncrement</source>
        <translation>视图旋转角度增量</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="87"/>
        <source>cappingOn</source>
        <translation>封口</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="88"/>
        <source>cappingHatchOn</source>
        <translation>封口填充</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="90"/>
        <source>color</source>
        <translation>颜色</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="91"/>
        <source>edgeColor</source>
        <translation>边颜色</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="92"/>
        <source>material</source>
        <translation>材质</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="93"/>
        <source>showEgesOn</source>
        <translation>显示边</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="94"/>
        <source>showNodesOn</source>
        <translation>显示节点</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="96"/>
        <source>autoExpandCompoundToAssembly</source>
        <translation>自动将复合体展开为装配体</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="17"/>
        <source>SI</source>
        <translation>国际单位制</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="18"/>
        <source>ImperialUK</source>
        <translation>英制（英国）</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="20"/>
        <source>VeryCoarse</source>
        <translation>非常粗糙</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="21"/>
        <source>Coarse</source>
        <translation>粗糙</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="22"/>
        <source>Normal</source>
        <translation>正常</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="23"/>
        <source>Precise</source>
        <translation>精确</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="24"/>
        <source>VeryPrecise</source>
        <translation>非常精确</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="25"/>
        <source>UserDefined</source>
        <translation>用户自定义</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="27"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="28"/>
        <source>ReloadIfUserConfirm</source>
        <translation>用户确认后重新加载</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="29"/>
        <source>ReloadSilently</source>
        <translation>静默重新加载</translation>
    </message>
</context>
<context>
    <name>Mayo::Application</name>
    <message>
        <location filename="../src/base/application.cpp" line="164"/>
        <source>Binary Mayo Document Format</source>
        <translation>Mayo 二进制文档格式</translation>
    </message>
    <message>
        <location filename="../src/base/application.cpp" line="171"/>
        <source>XML Mayo Document Format</source>
        <translation>Mayo XML 文档格式</translation>
    </message>
</context>
<context>
    <name>Mayo::BRepMeasureError</name>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="79"/>
        <source>Entity must be a vertex</source>
        <translation>实体必须是顶点</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="81"/>
        <source>Entity must be a circular edge</source>
        <translation>实体必须是圆形边</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="83"/>
        <source>Entity must be a shape(BREP)</source>
        <translation>实体必须是形状（BREP）</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="85"/>
        <source>Entity must be a geometric or polygon edge</source>
        <translation>实体必须是几何边或多边形边</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="87"/>
        <source>Entity must be a geometric or triangulation face</source>
        <translation>实体必须是几何面或三角剖分面</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="89"/>
        <source>Computation of minimum distance failed</source>
        <translation>最小距离计算失败</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="91"/>
        <source>Unable to find center of the shape</source>
        <translation>无法找到形状的中心</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="93"/>
        <source>All entities must be edges</source>
        <translation>所有实体必须是边</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="95"/>
        <source>Entity must be a linear edge</source>
        <translation>实体必须是线性边</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="97"/>
        <source>All entities must be faces</source>
        <translation>所有实体必须是面</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="99"/>
        <source>Entities must not be parallel</source>
        <translation>实体不能平行</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="101"/>
        <source>Bounding box computed is void</source>
        <translation>计算得到的包围盒为空</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="103"/>
        <source>Unknown error</source>
        <translation>未知错误</translation>
    </message>
</context>
<context>
    <name>Mayo::CliExport</name>
    <message>
        <location filename="../src/cli/cli_export.cpp" line="114"/>
        <source>Mesh BRep shapes</source>
        <translation>网格化 BRep 形状</translation>
    </message>
    <message>
        <location filename="../src/cli/cli_export.cpp" line="118"/>
        <source>Imported</source>
        <translation>已导入</translation>
    </message>
    <message>
        <location filename="../src/cli/cli_export.cpp" line="142"/>
        <source>Exported {}</source>
        <translation>已导出 {}</translation>
    </message>
    <message>
        <location filename="../src/cli/cli_export.cpp" line="222"/>
        <source>Importing...</source>
        <translation>正在导入...</translation>
    </message>
    <message>
        <location filename="../src/cli/cli_export.cpp" line="234"/>
        <source>Exporting {}...</source>
        <translation>正在导出 {}...</translation>
    </message>
</context>
<context>
    <name>Mayo::Command</name>
    <message>
        <location filename="../src/app/commands_display.cpp" line="39"/>
        <source>Orthographic</source>
        <translation>正交投影</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="40"/>
        <source>Perspective</source>
        <translation>透视投影</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="54"/>
        <source>Projection</source>
        <translation>投影</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="102"/>
        <source>Mode</source>
        <translation>模式</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="149"/>
        <source>[%1] %2</source>
        <translation>[%1] %2</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="173"/>
        <source>Show Origin Trihedron</source>
        <translation>显示原点三面体</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="174"/>
        <source>Show/Hide Origin Trihedron</source>
        <translation>显示/隐藏原点三面体</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="216"/>
        <source>Show Performance Stats</source>
        <translation>显示性能统计</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="217"/>
        <source>Show/Hide rendering performance statistics</source>
        <translation>显示/隐藏渲染性能统计</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="253"/>
        <source>Zoom In</source>
        <translation>放大</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="269"/>
        <source>Zoom Out</source>
        <translation>缩小</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="285"/>
        <source>Turn Counter Clockwise</source>
        <translation>逆时针旋转</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="302"/>
        <source>Turn Clockwise</source>
        <translation>顺时针旋转</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="52"/>
        <source>%1 files(%2)</source>
        <extracomment>%1 is the format identifier and %2 is the file filters string</extracomment>
        <translation>%1 文件（%2）</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="94"/>
        <source>All files(*.*)</source>
        <translation>所有文件（*.*）</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="96"/>
        <source>Select Part File</source>
        <translation>选择零件文件</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="192"/>
        <location filename="../src/app/commands_file.cpp" line="250"/>
        <source>Mesh BRep shapes</source>
        <translation>网格化 BRep 形状</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="197"/>
        <location filename="../src/app/commands_file.cpp" line="255"/>
        <source>Import time: {}ms</source>
        <translation>导入时间：{}毫秒</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="277"/>
        <source>New</source>
        <translation>新建</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="278"/>
        <source>New Document</source>
        <translation>新建文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="286"/>
        <source>Anonymous%1</source>
        <translation>未命名%1</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="293"/>
        <source>Open</source>
        <translation>打开</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="294"/>
        <source>Open Documents</source>
        <translation>打开文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="337"/>
        <source>Recent files</source>
        <translation>最近文件</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="364"/>
        <source>%1 | %2</source>
        <translation>%1 | %2</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="372"/>
        <source>Clear menu</source>
        <translation>清空菜单</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="259"/>
        <location filename="../src/app/commands_file.cpp" line="385"/>
        <source>Import</source>
        <translation>导入</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="386"/>
        <source>Import in current document</source>
        <translation>导入到当前文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="413"/>
        <location filename="../src/app/commands_file.cpp" line="414"/>
        <source>Export selected items</source>
        <translation>导出所选项目</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="422"/>
        <source>No item selected for export</source>
        <translation>未选择要导出的项目</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="434"/>
        <source>Select Output File</source>
        <translation>选择输出文件</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="461"/>
        <source>Export time: {}ms</source>
        <translation>导出时间：{}毫秒</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="478"/>
        <source>Close &quot;%1&quot;</source>
        <translation>关闭&quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="511"/>
        <source>Close %1</source>
        <translation>关闭 %1</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="512"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="522"/>
        <source>Close all</source>
        <translation>全部关闭</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="523"/>
        <source>Close all documents</source>
        <translation>关闭所有文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="540"/>
        <location filename="../src/app/commands_file.cpp" line="580"/>
        <source>Close all except current</source>
        <translation>关闭除当前外的所有文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="541"/>
        <source>Close all except current document</source>
        <translation>关闭除当前文档外的所有文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="579"/>
        <source>Close all except %1</source>
        <translation>关闭除 %1 外的所有文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="590"/>
        <source>Quit</source>
        <translation>退出</translation>
    </message>
    <message>
        <location filename="../src/app/commands_help.cpp" line="24"/>
        <source>Report Bug</source>
        <translation>报告错误</translation>
    </message>
    <message>
        <location filename="../src/app/commands_help.cpp" line="37"/>
        <source>About %1</source>
        <translation>关于 %1</translation>
    </message>
    <message>
        <location filename="../src/app/commands_tools.cpp" line="27"/>
        <location filename="../src/app/commands_tools.cpp" line="28"/>
        <source>Save View to Image</source>
        <translation>将视图保存为图像</translation>
    </message>
    <message>
        <location filename="../src/app/commands_tools.cpp" line="49"/>
        <location filename="../src/app/commands_tools.cpp" line="50"/>
        <source>Inspect XDE</source>
        <translation>检查 XDE</translation>
    </message>
    <message>
        <location filename="../src/app/commands_tools.cpp" line="88"/>
        <location filename="../src/app/commands_tools.cpp" line="89"/>
        <source>Options</source>
        <translation>选项</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="22"/>
        <source>Fullscreen</source>
        <translation>全屏</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="23"/>
        <source>Switch Fullscreen/Normal</source>
        <translation>切换全屏/普通模式</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="80"/>
        <source>Hide Left Sidebar</source>
        <translation>隐藏左侧栏</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="84"/>
        <source>Show Left Sidebar</source>
        <translation>显示左侧栏</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="51"/>
        <source>Show/Hide Left Sidebar</source>
        <translation>显示/隐藏左侧栏</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="95"/>
        <location filename="../src/app/commands_window.cpp" line="145"/>
        <source>Go To Home Page</source>
        <translation>转到主页</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="142"/>
        <source>Go To Documents</source>
        <translation>转到文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="156"/>
        <location filename="../src/app/commands_window.cpp" line="157"/>
        <source>Previous Document</source>
        <translation>上一个文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="181"/>
        <location filename="../src/app/commands_window.cpp" line="182"/>
        <source>Next Document</source>
        <translation>下一个文档</translation>
    </message>
    <message>
        <location filename="../src/app/command_system_information.cpp" line="51"/>
        <source>System Information...</source>
        <translation>系统信息...</translation>
    </message>
    <message>
        <location filename="../src/app/command_system_information.cpp" line="69"/>
        <source>Copy to Clipboard</source>
        <translation>复制到剪贴板</translation>
    </message>
</context>
<context>
    <name>Mayo::DialogAbout</name>
    <message>
        <location filename="../src/app/dialog_about.ui" line="14"/>
        <source>About</source>
        <translation>关于</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_about.ui" line="53"/>
        <source>Mayo By Fougue</source>
        <translation>Mayo — 由 Fougue 开发</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_about.ui" line="60"/>
        <source>Version %1 (%2bit)</source>
        <translation>版本 %1（%2位）</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_about.ui" line="67"/>
        <source>Built on %1 at %2</source>
        <translation>构建于 %1 %2</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_about.ui" line="92"/>
        <source>Qt %1</source>
        <translation>Qt %1</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_about.ui" line="99"/>
        <source>OpenCascade %1</source>
        <translation>OpenCascade %1</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_about.cpp" line="22"/>
        <source>%1 By %2</source>
        <translation>%1 — 由 %2 开发</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_about.cpp" line="42"/>
        <source>%1 %2</source>
        <translation>%1 %2</translation>
    </message>
</context>
<context>
    <name>Mayo::DialogInspectXde</name>
    <message>
        <location filename="../src/app/dialog_inspect_xde.ui" line="14"/>
        <source>XDE</source>
        <translation>XDE</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="171"/>
        <source>Yes</source>
        <translation>是</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="172"/>
        <source>No</source>
        <translation>否</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="378"/>
        <source>File Size: %1&lt;br&gt;Dimensions: %2x%3 Depth: %4</source>
        <translation>文件大小：%1&lt;br&gt;尺寸：%2x%3 深度：%4</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="383"/>
        <source>Error when loading texture file(invalid path?)</source>
        <translation>加载纹理文件时出错（路径无效？）</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="414"/>
        <source>%1,offset:%2</source>
        <translation>%1，偏移：%2</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="430"/>
        <source>&lt;data&gt;</source>
        <translation>&lt;数据&gt;</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="861"/>
        <source>Shape</source>
        <translation>形状</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="865"/>
        <source>Color</source>
        <translation>颜色</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="869"/>
        <source>Material</source>
        <translation>材质</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="874"/>
        <source>VisMaterial</source>
        <translation>可视化材质</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="879"/>
        <source>Dimension</source>
        <translation>尺寸</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="882"/>
        <source>Datum</source>
        <translation>基准</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="885"/>
        <source>GeomTolerance</source>
        <translation>几何公差</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="825"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="126"/>
        <source>ShapeType=%1, ShapeLocation=%2, Evolution=%3</source>
        <translation>形状类型=%1，形状位置=%2，演变=%3</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="825"/>
        <source>This document is not suitable for XDE</source>
        <translation>此文档不适用于 XDE</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="847"/>
        <source>Attributes</source>
        <translation>属性</translation>
    </message>
</context>
<context>
    <name>Mayo::DialogOptions</name>
    <message>
        <location filename="../src/app/dialog_options.ui" line="14"/>
        <source>Options</source>
        <translation>选项</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="148"/>
        <source>Restore default values</source>
        <translation>恢复默认值</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="183"/>
        <source>%1 / %2</source>
        <translation>%1 / %2</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="267"/>
        <source>Exchange</source>
        <translation>交换</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="270"/>
        <source>Load from file...</source>
        <translation>从文件加载...</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="271"/>
        <source>Save as...</source>
        <translation>另存为...</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="350"/>
        <location filename="../src/app/dialog_options.cpp" line="373"/>
        <source>Choose INI file</source>
        <translation>选择 INI 文件</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="350"/>
        <location filename="../src/app/dialog_options.cpp" line="373"/>
        <source>INI files(*.ini)</source>
        <translation>INI 文件（*.ini）</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="357"/>
        <location filename="../src/app/dialog_options.cpp" line="362"/>
        <location filename="../src/app/dialog_options.cpp" line="382"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="357"/>
        <source>&apos;%1&apos; doesn&apos;t exist</source>
        <translation>&quot;%1&quot;不存在</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="362"/>
        <source>&apos;%1&apos; is not readable</source>
        <translation>&quot;%1&quot;不可读</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="382"/>
        <source>Error when writing to &apos;%1&apos;</source>
        <translation>写入&quot;%1&quot;时出错</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="408"/>
        <source>Restore values for default section only</source>
        <translation>仅恢复默认部分的值</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="411"/>
        <source>Restore values for the whole group</source>
        <translation>恢复整个组的值</translation>
    </message>
</context>
<context>
    <name>Mayo::DialogSaveImageView</name>
    <message>
        <location filename="../src/app/dialog_save_image_view.ui" line="14"/>
        <source>Save View to Image</source>
        <translation>将视图保存为图像</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.ui" line="23"/>
        <source>Options</source>
        <translation>选项</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.ui" line="29"/>
        <source>Width</source>
        <translation>宽度</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.ui" line="36"/>
        <location filename="../src/app/dialog_save_image_view.ui" line="56"/>
        <source>px</source>
        <translation>像素</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.ui" line="49"/>
        <source>Height</source>
        <translation>高度</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.ui" line="69"/>
        <location filename="../src/app/dialog_save_image_view.cpp" line="119"/>
        <source>Keep ratio</source>
        <translation>保持比例</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="43"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="44"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="45"/>
        <source>Preview</source>
        <translation>预览</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="68"/>
        <source>%1 files(*.%2)</source>
        <translation>%1 文件（*.%2）</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="75"/>
        <source>Select image file</source>
        <translation>选择图像文件</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="94"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="94"/>
        <source>Failed to save image &apos;%1&apos;</source>
        <translation>保存图像&quot;%1&quot;失败</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="116"/>
        <source>%1x%2 %3</source>
        <translation>%1x%2 %3</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="119"/>
        <source>Free ratio</source>
        <translation>自由比例</translation>
    </message>
</context>
<context>
    <name>Mayo::DialogTaskManager</name>
    <message>
        <location filename="../src/app/dialog_task_manager.ui" line="14"/>
        <source>Tasks</source>
        <translation>任务</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_task_manager.cpp" line="189"/>
        <source> / </source>
        <translation> / </translation>
    </message>
</context>
<context>
    <name>Mayo::DocumentPropertyGroup</name>
    <message>
        <location filename="../src/app/document_property_group.h" line="20"/>
        <source>filepath</source>
        <translation>文件路径</translation>
    </message>
    <message>
        <location filename="../src/app/document_property_group.h" line="21"/>
        <source>fileSize</source>
        <translation>文件大小</translation>
    </message>
    <message>
        <location filename="../src/app/document_property_group.h" line="22"/>
        <source>createdDateTime</source>
        <translation>创建时间</translation>
    </message>
    <message>
        <location filename="../src/app/document_property_group.h" line="23"/>
        <source>modifiedDateTime</source>
        <translation>修改时间</translation>
    </message>
    <message>
        <location filename="../src/app/document_property_group.h" line="24"/>
        <source>owner</source>
        <translation>所有者</translation>
    </message>
    <message>
        <location filename="../src/app/document_property_group.h" line="25"/>
        <source>entityCount</source>
        <translation>实体数量</translation>
    </message>
</context>
<context>
    <name>Mayo::GraphicsMeshObjectDriver</name>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="36"/>
        <source>Wireframe</source>
        <translation>线框</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="37"/>
        <source>Shaded</source>
        <translation>着色</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="38"/>
        <source>Shrink</source>
        <translation>收缩</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="200"/>
        <source>color</source>
        <translation>颜色</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="201"/>
        <source>edgeColor</source>
        <translation>边颜色</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="202"/>
        <source>showEdges</source>
        <translation>显示边</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="203"/>
        <source>showNodes</source>
        <translation>显示节点</translation>
    </message>
</context>
<context>
    <name>Mayo::GraphicsObjectDriverI18N</name>
    <message>
        <location filename="messages.cpp" line="36"/>
        <source>GraphicsShapeObjectDriver</source>
        <translation>形状</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="37"/>
        <source>GraphicsMeshObjectDriver</source>
        <translation>网格</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="38"/>
        <source>GraphicsPointCloudObjectDriver</source>
        <translation>点云</translation>
    </message>
</context>
<context>
    <name>Mayo::GraphicsShapeObjectDriver</name>
    <message>
        <location filename="../src/graphics/graphics_shape_object_driver.cpp" line="32"/>
        <source>Wireframe</source>
        <translation>线框</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_shape_object_driver.cpp" line="33"/>
        <source>HiddenLineRemoval</source>
        <translation>消隐线</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_shape_object_driver.cpp" line="34"/>
        <source>Shaded</source>
        <translation>着色</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_shape_object_driver.cpp" line="35"/>
        <source>ShadedWithFaceBoundary</source>
        <translation>带边着色</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::AssimpReaderI18N</name>
    <message>
        <location filename="../src/io_assimp/io_assimp_reader.cpp" line="365"/>
        <source>LINE primitives not supported yet</source>
        <translation>尚不支持 LINE 基元</translation>
    </message>
    <message>
        <location filename="../src/io_assimp/io_assimp_reader.cpp" line="368"/>
        <source>Some primitive not supported</source>
        <translation>某些基元不受支持</translation>
    </message>
    <message>
        <location filename="../src/io_assimp/io_assimp_reader.cpp" line="499"/>
        <source>Texture not found: {}
Tried:</source>
        <translation>找不到纹理：{}
已尝试：</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::DxfReader::Properties</name>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="543"/>
        <source>Import text/dimension objects</source>
        <translation>导入文本/尺寸对象</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="545"/>
        <source>Group all objects within a layer into a single compound shape</source>
        <translation>将图层中的所有对象分组为单一复合形状</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="547"/>
        <source>Name of the font to be used when creating shape for text objects</source>
        <translation>为文本对象创建形状时使用的字体名称</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="557"/>
        <source>importAnnotations</source>
        <translation>导入注释</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="558"/>
        <source>groupLayers</source>
        <translation>按图层分组对象</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="559"/>
        <source>fontNameForTextObjects</source>
        <translation>文本对象字体</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::GmioAmfWriter::Properties</name>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="134"/>
        <source>Decimal floating point(ex: 392.65)</source>
        <translation>十进制浮点数（例：392.65）</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="135"/>
        <source>Scientific notation(ex: 3.9265E+2)</source>
        <translation>科学计数法（例：3.9265E+2）</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="136"/>
        <source>Use the shortest representation: decimal or scientific</source>
        <translation>使用最短表示形式：十进制或科学计数法</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="132"/>
        <source>Format used when writing `double` values as strings</source>
        <translation>将 `double` 值写为字符串时使用的格式</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="142"/>
        <source>Maximum number of significant digits when writing `double` values</source>
        <translation>写入 `double` 值时的最大有效位数</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="145"/>
        <source>Write AMF document in ZIP archive containing one file entry</source>
        <translation>将 AMF 文档写入包含单个文件条目的 ZIP 压缩包</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="148"/>
        <source>Filename of the single AMF entry within the ZIP archive.
Only applicable if option `{}` is on</source>
        <translation>ZIP 压缩包中唯一 AMF 条目的文件名。
仅在选项 `{}` 开启时适用</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="153"/>
        <source>Use the ZIP64 format extensions.
Only applicable if option `{}` is on</source>
        <translation>使用 ZIP64 格式扩展。
仅在选项 `{}` 开启时适用</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="181"/>
        <source>float64Format</source>
        <translation>64位浮点格式</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="182"/>
        <source>float64Precision</source>
        <translation>64位浮点精度</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="183"/>
        <source>createZipArchive</source>
        <translation>创建 ZIP 压缩包</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="184"/>
        <source>zipEntryFilename</source>
        <translation>ZIP 条目文件名</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="185"/>
        <source>useZip64</source>
        <translation>使用 ZIP64 扩展</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="41"/>
        <source>Decimal</source>
        <translation>十进制</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="42"/>
        <source>Scientific</source>
        <translation>科学计数法</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="43"/>
        <source>Shortest</source>
        <translation>最短形式</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::ImageWriterI18N</name>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="51"/>
        <source>Image width in pixels</source>
        <translation>图像宽度（像素）</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="55"/>
        <source>Image height in pixels</source>
        <translation>图像高度（像素）</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="59"/>
        <source>Start color of the image background gradient</source>
        <translation>图像背景渐变的起始颜色</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="62"/>
        <source>End color of the image background gradient</source>
        <translation>图像背景渐变的结束颜色</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="65"/>
        <source>Type of gradient fill for the image background</source>
        <translation>图像背景的渐变填充类型</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="69"/>
        <source>No gadient fill, single color background</source>
        <translation>无渐变填充，单色背景</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="70"/>
        <source>Gradient directed from left to right</source>
        <translation>从左到右的渐变</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="71"/>
        <source>Gradient directed from top to bottom</source>
        <translation>从上到下的渐变</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="72"/>
        <source>Gradient directed from top left corner to bottom right</source>
        <translation>从左上角到右下角的渐变</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="73"/>
        <source>Gradient directed from top right corner to bottom left</source>
        <translation>从右上角到左下角的渐变</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="74"/>
        <source>Gradient directed from center in all directions forming concentric circles</source>
        <translation>从中心向四周辐射形成同心圆的渐变</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="78"/>
        <source>Camera orientation expressed in Z-up convention as a unit vector</source>
        <translation>以 Z 轴朝上约定表示的相机朝向单位向量</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="81"/>
        <source>Camera projection type, specifies how the 3D scene is projected onto a 2D image for display</source>
        <translation>相机投影类型，指定 3D 场景如何投影到 2D 图像上进行显示</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="101"/>
        <source>Graphics display mode for the objects of type `{}`</source>
        <translation>类型为 `{}` 的对象的图形显示模式</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="128"/>
        <source>width</source>
        <translation>宽度</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="129"/>
        <source>height</source>
        <translation>高度</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="130"/>
        <source>backgroundColorStart</source>
        <translation>背景颜色起点</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="131"/>
        <source>backgroundColorEnd</source>
        <translation>背景颜色终点</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="132"/>
        <source>backgroundGradientFill</source>
        <translation>背景渐变填充</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="195"/>
        <source>Background radial gradient fill is available since OpenCascade 7.6.
Default to background single color</source>
        <translation>背景径向渐变填充自 OpenCascade 7.6 起可用。
默认为背景单色</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="133"/>
        <source>cameraOrientation</source>
        <translation>相机朝向</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="134"/>
        <source>cameraProjection</source>
        <translation>相机投影</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="183"/>
        <source>No transferred application items</source>
        <translation>没有已传输的应用程序项目</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="191"/>
        <source>Camera orientation vector must not be null</source>
        <translation>相机朝向向量不得为零向量</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="68"/>
        <source>Perspective</source>
        <translation>透视</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="69"/>
        <source>Orthographic</source>
        <translation>正交</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="71"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="72"/>
        <source>Horizontal</source>
        <translation>水平</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="73"/>
        <source>Vertical</source>
        <translation>垂直</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="74"/>
        <source>DiagonalTopLeftBottomRight</source>
        <translation>对角线（左上 → 右下）</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="75"/>
        <source>DiagonalTopRightBottomLeft</source>
        <translation>对角线（右上 → 左下）</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="76"/>
        <source>Radial</source>
        <translation>径向</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="77"/>
        <source>GraphicsShapeObjectDriver_displayMode</source>
        <translation>形状对象的显示模式</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="78"/>
        <source>GraphicsMeshObjectDriver_displayMode</source>
        <translation>网格对象的显示模式</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccBaseMeshReaderProperties</name>
    <message>
        <location filename="../src/io_occ/io_occ_base_mesh.cpp" line="27"/>
        <source>rootPrefix</source>
        <translation>根标签前缀</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_base_mesh.cpp" line="28"/>
        <source>systemCoordinatesConverter</source>
        <translation>系统坐标转换器</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_base_mesh.cpp" line="29"/>
        <source>systemLengthUnit</source>
        <translation>系统长度单位</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_base_mesh.cpp" line="31"/>
        <source>Prefix for generating root labels name</source>
        <translation>用于生成根标签名称的前缀</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_base_mesh.cpp" line="32"/>
        <source>System length units to convert into while reading files</source>
        <translation>读取文件时转换为的系统长度单位</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccCommon</name>
    <message>
        <location filename="messages.cpp" line="45"/>
        <location filename="messages.cpp" line="49"/>
        <source>Undefined</source>
        <translation>未定义</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="46"/>
        <source>posYfwd_posZup</source>
        <translation>+Z 朝上</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="47"/>
        <source>negZfwd_posYup</source>
        <translation>+Y 朝上</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="50"/>
        <source>Micrometer</source>
        <translation>微米</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="51"/>
        <source>Millimeter</source>
        <translation>毫米</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="52"/>
        <source>Centimeter</source>
        <translation>厘米</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="53"/>
        <source>Meter</source>
        <translation>米</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="54"/>
        <source>Kilometer</source>
        <translation>千米</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="55"/>
        <source>Inch</source>
        <translation>英寸</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="56"/>
        <source>Foot</source>
        <translation>英尺</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="57"/>
        <source>Mile</source>
        <translation>英里</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccGltfReader::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_reader.cpp" line="32"/>
        <source>skipEmptyNodes</source>
        <translation>跳过空节点</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_reader.cpp" line="33"/>
        <source>useMeshNameAsFallback</source>
        <translation>使用网格名称作为备用</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_reader.cpp" line="18"/>
        <source>Ignore nodes without geometry(`Yes` by default)</source>
        <translation>忽略没有几何体的节点（默认为&quot;是&quot;）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_reader.cpp" line="21"/>
        <source>Use mesh name in case if node name is empty(`Yes` by default)</source>
        <translation>当节点名称为空时使用网格名称（默认为&quot;是&quot;）</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccGltfWriter::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="107"/>
        <source>transformationFormat</source>
        <translation>变换格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="108"/>
        <source>format</source>
        <translation>目标格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="109"/>
        <source>forceExportUV</source>
        <translation>强制导出 UV</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="30"/>
        <source>Source coordinate system transformation</source>
        <translation>源坐标系变换</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="32"/>
        <source>Target coordinate system transformation</source>
        <translation>目标坐标系变换</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="34"/>
        <source>Preferred transformation format for writing into glTF file</source>
        <translation>写入 glTF 文件时首选的变换格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="36"/>
        <source>Export UV coordinates even if there is no mapped texture</source>
        <translation>即使没有映射纹理也导出 UV 坐标</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="40"/>
        <source>Automatically choose most compact representation between Mat4 and TRS</source>
        <translation>自动在 Mat4 和 TRS 之间选择最紧凑的表示形式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="42"/>
        <source>4x4 transformation matrix</source>
        <translation>4x4 变换矩阵</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="43"/>
        <source>Transformation decomposed into Translation vector, Rotation quaternion and Scale factor(T * R * S)</source>
        <translation>变换分解为平移向量、旋转四元数和缩放因子（T * R * S）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="49"/>
        <source>Name format for exporting nodes</source>
        <translation>导出节点的名称格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="50"/>
        <source>Name format for exporting meshes</source>
        <translation>导出网格的名称格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="52"/>
        <source>Write image textures into target file.

If set to `false` then texture images will be written as separate files.

Applicable only if option `{0}` is set to `{1}`</source>
        <translation>将图像纹理写入目标文件。

若设为 `false`，纹理图像将写为单独的文件。

仅在选项 `{0}` 设为 `{1}` 时适用</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="60"/>
        <source>Merge faces within a single part.

May reduce JSON size thanks to smaller number of primitive arrays</source>
        <translation>合并单个零件内的面。

可减少基元数组数量，从而缩小 JSON 大小</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="64"/>
        <source>Prefer keeping 16-bit indexes while merging face.

May reduce binary data size thanks to smaller triangle indexes.

Applicable only if option `{}` is on</source>
        <translation>合并面时优先保留 16 位索引。

可通过减小三角形索引来缩小二进制数据大小。

仅在选项 `{}` 开启时适用</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="105"/>
        <source>inputCoordinateSystem</source>
        <translation>输入坐标系</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="106"/>
        <source>outputCoordinateSystem</source>
        <translation>输出坐标系</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="110"/>
        <source>nodeNameFormat</source>
        <translation>节点名称格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="111"/>
        <source>meshNameFormat</source>
        <translation>网格名称格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="112"/>
        <source>embedTextures</source>
        <translation>嵌入纹理</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="113"/>
        <source>mergeFaces</source>
        <translation>合并面</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="114"/>
        <source>keepIndices16b</source>
        <translation>保留 16 位索引</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="170"/>
        <source>Option supported from OpenCascade ≥ v7.6 [option={}, actual version={}]</source>
        <translation>此选项自 OpenCascade ≥ v7.6 起支持 [选项={}，当前版本={}]</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="62"/>
        <source>Json</source>
        <translation>JSON</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="63"/>
        <source>Binary</source>
        <translation>二进制</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccIgesReader::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="88"/>
        <source>bsplineContinuity</source>
        <translation>B 样条连续性</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="89"/>
        <source>surfaceCurveMode</source>
        <translation>曲面曲线模式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="90"/>
        <source>readFaultyEntities</source>
        <translation>读取错误实体</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="91"/>
        <source>readOnlyVisibleEntities</source>
        <translation>仅读取可见实体</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="56"/>
        <source>Read failed entities</source>
        <translation>读取失败的实体</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="26"/>
        <source>Manages the continuity of BSpline curves (IGES entities 106, 112 and 126) after translation to Open CASCADE (it requires that the curves in a model be at least C1 continuous; no such requirement is made by IGES).This parameter does not change the continuity of curves that are used in the construction of IGES BRep entities. In this case, the parameter does not influence the continuity of the resulting Open CASCADE curves (it is ignored).</source>
        <translation>管理 BSpline 曲线（IGES 实体 106、112 和 126）在转换为 Open CASCADE 后的连续性。
该过程要求模型中的曲线至少满足 C1 连续性；而 IGES 标准并没有这一要求。

该参数不会改变用于构建 IGES BRep 实体的曲线连续性。在这种情况下，该参数不会影响最终 Open CASCADE 曲线的连续性（将被忽略）。</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="37"/>
        <source>Preference for the computation of curves in case of 2D/3D inconsistency in an entity which has both 2D and 3D representations.

Concerned entity types are 141 (Boundary), 142 (CurveOnSurface) and 508 (Loop). These are entities representing a contour lying on a surface, which is translated to a TopoDS_Wire, formed by TopoDS_Edges. Each TopoDS_Edge must have a 3D curve and a 2D curve that reference the surface.

The processor also decides to re-compute either the 3D or the 2D curve even if both curves are translated successfully and seem to be correct, in case there is inconsistency between them. The processor considers that there is inconsistency if any of the following conditions is satisfied:
- the number of sub-curves in the 2D curve is different from the number of sub-curves in the 3D curve. This can be either due to different numbers of sub-curves given in the IGES file or because of splitting of curves during translation
- 3D or 2D curve is a Circular Arc (entity type 100) starting and ending in the same point (note that this case is incorrect according to the IGES standard)</source>
        <translation>在 2D/3D 表示不一致的情况下，对曲线计算方式的偏好设置（适用于同时具有 2D 和 3D 表示的实体）。

相关实体类型包括 141（边界）、142（曲面曲线）和 508（环）。这些实体表示位于曲面上的轮廓，并被转换为由 TopoDS_Edge 构成的 TopoDS_Wire。每个 TopoDS_Edge 必须同时具有引用该曲面的 3D 曲线和 2D 曲线。

当 2D 和 3D 曲线之间存在不一致时，即使两者均已成功转换且看似正确，处理器也可能选择重新计算 3D 或 2D 曲线。

当满足以下任一条件时，处理器认为存在不一致：
- 2D 曲线中的子曲线数量与 3D 曲线中的子曲线数量不同。这可能是由于 IGES 文件中定义的子曲线数量不同，或在转换过程中发生曲线分割所致；
- 3D 或 2D 曲线为圆弧（实体类型 100），且其起点与终点相同（该情况根据 IGES 标准是错误的</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="59"/>
        <source>Curves are taken as they are in the IGES file. C0 entities of Open CASCADE may be produced</source>
        <translation>曲线保持 IGES 文件中的原样。可能生成 Open CASCADE 的 C0 实体</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="62"/>
        <source>If an IGES BSpline, Spline or CopiousData curve is C0 continuous, it is broken down into pieces of C1 continuous Geom_BSplineCurve</source>
        <translation>若 IGES B 样条、样条或密集数据曲线为 C0 连续，则将其分解为 C1 连续的 Geom_BSplineCurve 片段</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="65"/>
        <source>IGES Spline curves are broken down into pieces of C2 continuity. If C2 cannot be ensured, the Spline curves will be broken down into pieces of C1 continuity</source>
        <translation>IGES 样条曲线分解为 C2 连续性片段。若无法保证 C2，则分解为 C1 连续性片段</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="72"/>
        <source>Use the preference flag value in the entity&apos;s `Parameter Data` section</source>
        <translation>使用实体&quot;参数数据&quot;部分中的首选项标志值</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="73"/>
        <source>The 2D is used to rebuild the 3D in case of their inconsistency</source>
        <translation>当 2D 与 3D 不一致时，使用 2D 重建 3D</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="74"/>
        <source>The 2D is always used to rebuild the 3D (even if 3D is present in the file)</source>
        <translation>始终使用 2D 重建 3D（即使文件中存在 3D）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="75"/>
        <source>The 3D is used to rebuild the 2D in case of their inconsistency</source>
        <translation>当 3D 与 2D 不一致时，使用 3D 重建 2D</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="76"/>
        <source>The 3D is always used to rebuild the 2D (even if 2D is present in the file)</source>
        <translation>始终使用 3D 重建 2D（即使文件中存在 2D）</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccIgesWriter::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="176"/>
        <source>brepMode</source>
        <translation>BRep 模式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="177"/>
        <source>planeMode</source>
        <translation>平面模式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="178"/>
        <source>lengthUnit</source>
        <translation>长度单位</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="156"/>
        <source>Indicates if planes should be saved as Bsplines or Planes (type 108). Writing p-curves on planes is disabled</source>
        <translation>指示平面应保存为 B 样条还是平面（类型 108）。禁用在平面上写入参数曲线</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="160"/>
        <source>OpenCascade TopoDS_Faces will be translated into IGES 144 (Trimmed Surface) entities, no BRep entities will be written to the IGES file</source>
        <translation>OpenCascade TopoDS_Faces 将转换为 IGES 144（修剪曲面）实体，不向 IGES 文件写入 BRep 实体</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="163"/>
        <source>OpenCascade TopoDS_Faces will be translated into IGES 510 (Face) entities, the IGES file will contain BRep entities</source>
        <translation>OpenCascade TopoDS_Faces 将转换为 IGES 510（面）实体，IGES 文件将包含 BRep 实体</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccObjReader::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_obj_reader.cpp" line="28"/>
        <source>singlePrecisionVertexCoords</source>
        <translation>顶点坐标单精度</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_obj_reader.cpp" line="18"/>
        <source>Single precision flag for reading vertex data(coordinates)</source>
        <translation>读取顶点数据（坐标）时使用单精度的标志</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccObjWriterI18N</name>
    <message>
        <location filename="../src/io_occ/io_occ_obj_writer.cpp" line="30"/>
        <source>Source coordinate system transformation</source>
        <translation>源坐标系变换</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_obj_writer.cpp" line="33"/>
        <source>Target coordinate system transformation</source>
        <translation>目标坐标系变换</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_obj_writer.cpp" line="43"/>
        <source>inputCoordinateSystem</source>
        <translation>输入坐标系</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_obj_writer.cpp" line="44"/>
        <source>outputCoordinateSystem</source>
        <translation>输出坐标系</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccStepReader::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="118"/>
        <source>productContext</source>
        <translation>产品上下文</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="119"/>
        <source>assemblyLevel</source>
        <translation>装配层级</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="120"/>
        <source>preferredShapeRepresentation</source>
        <translation>首选形状表示</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="121"/>
        <source>readShapeAspect</source>
        <translation>读取形状外观</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="122"/>
        <source>readSubShapesNames</source>
        <translation>读取子形状名称</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="123"/>
        <source>encoding</source>
        <translation>编码</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="68"/>
        <source>Translate only products that have `PRODUCT_DEFINITION_CONTEXT` with field `life_cycle_stage` set to `design`</source>
        <translation>仅转换 `PRODUCT_DEFINITION_CONTEXT` 中 `life_cycle_stage` 字段设为 `design` 的产品</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="35"/>
        <source>When reading AP 209 STEP files, allows selecting either only `design` or `analysis`, or both types of products for translation
Note that in AP 203 and AP214 files all products should be marked as `design`, so if this mode is set to `analysis`, nothing will be read</source>
        <translation>在读取 AP 209 STEP 文件时，可以选择仅翻译 `design`（设计）类型产品、仅翻译 `analysis`（分析）类型产品，或同时翻译两种类型的产品。

注意，在 AP 203 和 AP 214 文件中，所有产品都应被标记为 `design`。因此，如果该模式设置为 `analysis`，将不会读取任何内容</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="42"/>
        <source>Specifies which data should be read for the products found in the STEP file</source>
        <translation>指定应为 STEP 文件中找到的产品读取哪些数据</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="46"/>
        <source>Specifies preferred type of representation of the shape of the product, in case if a STEP file contains more than one representation (i.e. multiple `PRODUCT_DEFINITION_SHAPE` entities) for a single product</source>
        <translation>指定产品形状的首选表示类型，用于当 STEP 文件中同一产品存在多个表示（即多个 `PRODUCT_DEFINITION_SHAPE` 实体）时</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="52"/>
        <source>Defines whether shapes associated with the `PRODUCT_DEFINITION_SHAPE` entity of the product via `SHAPE_ASPECT` should be translated.
This kind of association was used for the representation of hybrid models (i.e. models whose shape is composed of different types of representations) in AP 203 files before 1998, but it is also used to associate auxiliary information with the sub-shapes of the part. Though STEP translator tries to recognize such cases correctly, this parameter may be useful to avoid unconditionally translation of shapes associated via `SHAPE_ASPECT` entities.</source>
        <translation>定义是否应翻译通过 `SHAPE_ASPECT` 与产品的 `PRODUCT_DEFINITION_SHAPE` 实体相关联的形状。

这种关联方式曾用于表示混合模型（即形状由不同类型表示组成的模型），在 1998 年之前的 AP 203 文件中较为常见，同时也用于将辅助信息关联到零件的子形状上。尽管 STEP 转换器会尝试正确识别这些情况，但该参数仍可用于避免对通过 `SHAPE_ASPECT` 实体关联的形状进行无条件转换。</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="63"/>
        <source>Indicates whether to read sub-shape names from &apos;Name&apos; attributes of STEP Representation Items</source>
        <translation>指示是否从 STEP 表示项（Representation Items）的 `Name` 属性中读取子形状名称</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="71"/>
        <source>Translate only products that have `PRODUCT_DEFINITION_CONTEXT` with field `life_cycle_stage` set to `analysis`</source>
        <translation>仅转换 `PRODUCT_DEFINITION_CONTEXT` 中 `life_cycle_stage` 字段设为 `analysis` 的产品</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="74"/>
        <source>Translates all products</source>
        <translation>转换所有产品</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="78"/>
        <source>Translate the assembly structure and shapes associated with parts only(not with sub-assemblies)</source>
        <translation>仅转换装配结构和与零件关联的形状（不包括子装配体）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="81"/>
        <source>Translate only the assembly structure without shapes(a structure of empty compounds). This mode can be useful as an intermediate step in applications requiring specialized processing of assembly parts</source>
        <translation>仅转换装配结构而不含形状（空复合体结构）。此模式可作为需要对装配零件进行专项处理的应用程序的中间步骤</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="85"/>
        <source>Translate only shapes associated with the product, ignoring the assembly structure (if any). This can be useful to translate only a shape associated with specific product, as a complement to assembly mode</source>
        <translation>仅转换与产品关联的形状，忽略装配结构（如有）。可用于仅转换与特定产品关联的形状，作为装配模式的补充</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="89"/>
        <source>Translate both the assembly structure and all associated shapes. If both shape and sub-assemblies are associated with the same product, all of them are read and put in a single compound</source>
        <translation>同时转换装配结构和所有关联形状。若形状和子装配体都与同一产品关联，则全部读取并放入单一复合体</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="97"/>
        <source>Translate all representations(if more than one, put in compound)</source>
        <translation>转换所有表示（若多于一个，则放入复合体）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="101"/>
        <source>Shift Japanese Industrial Standards</source>
        <translation>移位日本工业标准（Shift-JIS）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="102"/>
        <source>EUC(Extended Unix Code), multi-byte encoding primarily for Japanese, Korean, and simplified Chinese</source>
        <translation>EUC（扩展 Unix 编码），主要用于日语、韩语和简体中文的多字节编码</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="104"/>
        <source>GB(Guobiao) encoding for Simplified Chinese</source>
        <translation>GB（国标）简体中文编码</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccStepWriter::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="289"/>
        <source>schema</source>
        <translation>模式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="290"/>
        <source>lengthUnit</source>
        <translation>长度单位</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="291"/>
        <source>assemblyMode</source>
        <translation>装配模式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="292"/>
        <source>freeVertexMode</source>
        <translation>自由顶点模式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="293"/>
        <source>writeParametericCurves</source>
        <translation>写入参数曲线</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="294"/>
        <source>writeSubShapesNames</source>
        <translation>写入子形状名称</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="295"/>
        <source>headerAuthor</source>
        <translation>作者（标头）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="296"/>
        <source>headerOrganization</source>
        <translation>组织（标头）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="297"/>
        <source>headerOriginatingSystem</source>
        <translation>原始系统（标头）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="298"/>
        <source>headerDescription</source>
        <translation>描述（标头）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="235"/>
        <source>Version of schema used for the output STEP file</source>
        <translation>输出 STEP 文件所用的模式版本</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="238"/>
        <source>Defines a unit in which the STEP file should be written. If set to unit other than millimeter, the model is converted to these units during the translation</source>
        <translation>定义 STEP 文件的写入单位。若设为毫米以外的单位，模型在转换过程中将转换为该单位</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="244"/>
        <source>Parameter to write all free vertices in one SDR (name and style of vertex are lost) or each vertex in its own SDR (name and style of vertex are exported)</source>
        <translation>将所有自由顶点写入一个 SDR（顶点名称和样式丢失）或每个顶点写入自己的 SDR（导出顶点名称和样式）的参数</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="249"/>
        <source>All free vertices are united into one compound and exported in one shape definition representation (vertex name and style are lost)</source>
        <translation>所有自由顶点合并为一个复合体并以单一形状定义表示导出（顶点名称和样式丢失）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="252"/>
        <source>Each vertex is exported in its own `SHAPE DEFINITION REPRESENTATION`(vertex name and style are not lost, but the STEP file size increases)</source>
        <translation>每个顶点在其自己的&quot;形状定义表示&quot;中导出（顶点名称和样式不丢失，但 STEP 文件大小增加）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="258"/>
        <source>Indicates whether parametric curves (curves in parametric space of surface) should be written into the STEP file.
It can be disabled in order to minimize the size of the resulting file.</source>
        <translation>指示是否应将参数曲线（曲面参数空间中的曲线）写入 STEP 文件。
可禁用以最小化输出文件大小。</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="264"/>
        <source>Indicates whether to write sub-shape names to &apos;Name&apos; attributes of STEP Representation Items</source>
        <translation>指示是否将子形状名称写入 STEP 表示项的&quot;名称&quot;属性</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="268"/>
        <source>Author attribute in STEP header</source>
        <translation>STEP 标头中的作者属性</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="269"/>
        <source>Organization(of author) attribute in STEP header</source>
        <translation>STEP 标头中的组织（作者所属）属性</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="270"/>
        <source>Originating system attribute in STEP header</source>
        <translation>STEP 标头中的原始系统属性</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="271"/>
        <source>Description attribute in STEP header</source>
        <translation>STEP 标头中的描述属性</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccStlWriterI18N</name>
    <message>
        <location filename="../src/io_occ/io_occ_stl.cpp" line="68"/>
        <source>targetFormat</source>
        <translation>目标格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_stl.cpp" line="120"/>
        <location filename="../src/io_occ/io_occ_stl.cpp" line="123"/>
        <source>Not all BRep faces are meshed</source>
        <translation>并非所有 BRep 面都已网格化</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="59"/>
        <source>Ascii</source>
        <translation>文本</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="60"/>
        <source>Binary</source>
        <translation>二进制</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccVrmlWriter::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_vrml_writer.cpp" line="41"/>
        <source>shapeRepresentation</source>
        <translation>形状表示</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OffReaderI18N</name>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="164"/>
        <source>Can&apos;t open input file</source>
        <translation>无法打开输入文件</translation>
    </message>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="172"/>
        <location filename="../src/io_off/io_off_reader.cpp" line="192"/>
        <source>Unexpected end of file</source>
        <translation>意外的文件结尾</translation>
    </message>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="176"/>
        <source>Wrong header keyword(should be [C][N][4]OFF</source>
        <translation>错误的标头关键字（应为 [C][N][4]OFF）</translation>
    </message>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="197"/>
        <source>No vertex or face count</source>
        <translation>无顶点或面计数</translation>
    </message>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="218"/>
        <source>No vertex coordinates at current line</source>
        <translation>当前行没有顶点坐标</translation>
    </message>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="243"/>
        <source>Inconsistent vertex count of face</source>
        <translation>面的顶点数不一致</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OffWriterI18N</name>
    <message>
        <location filename="../src/io_off/io_off_writer.cpp" line="44"/>
        <source>Failed to open file</source>
        <translation>打开文件失败</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::PlyWriterI18N</name>
    <message>
        <location filename="messages.cpp" line="65"/>
        <source>Ascii</source>
        <translation>文本</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="66"/>
        <source>Binary</source>
        <translation>二进制</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::System</name>
    <message>
        <location filename="../src/base/io_system.cpp" line="234"/>
        <source>Reading file</source>
        <translation>正在读取文件</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="228"/>
        <source>Unknown format</source>
        <translation>未知格式</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="218"/>
        <source>Error during import of &apos;{}&apos;
{}</source>
        <translation>导入&quot;{}&quot;时出错
{}</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="237"/>
        <source>No supporting reader</source>
        <translation>没有支持的读取器</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="247"/>
        <source>File read problem</source>
        <translation>文件读取问题</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="256"/>
        <source>Transferring file</source>
        <translation>正在传输文件</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="260"/>
        <location filename="../src/base/io_system.cpp" line="386"/>
        <source>File transfer problem</source>
        <translation>文件传输问题</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="378"/>
        <source>No supporting writer</source>
        <translation>没有支持的写入器</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="383"/>
        <source>Transfer</source>
        <translation>传输</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="390"/>
        <source>Write</source>
        <translation>写入</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="393"/>
        <source>File write problem</source>
        <translation>文件写入问题</translation>
    </message>
</context>
<context>
    <name>Mayo::Main</name>
    <message>
        <location filename="../src/app/main.cpp" line="99"/>
        <source>Theme for the UI(classic|dark)</source>
        <translation>界面主题（classic|dark）</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="100"/>
        <source>name</source>
        <translation>名称</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="113"/>
        <location filename="../src/cli/main.cpp" line="229"/>
        <source>Writes log messages into output file</source>
        <translation>将日志消息写入输出文件</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="120"/>
        <location filename="../src/cli/main.cpp" line="236"/>
        <source>Don&apos;t filter out debug log messages in release build</source>
        <translation>在发布版本中不过滤调试日志消息</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="126"/>
        <location filename="../src/cli/main.cpp" line="248"/>
        <source>Show detailed system information and quit</source>
        <translation>显示详细的系统信息并退出</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="131"/>
        <location filename="../src/cli/main.cpp" line="253"/>
        <source>files</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="132"/>
        <source>Files to open at startup, optionally</source>
        <translation>启动时可选择打开的文件</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="133"/>
        <location filename="../src/cli/main.cpp" line="255"/>
        <source>[files...]</source>
        <translation>[文件...]</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="174"/>
        <location filename="../src/cli/main.cpp" line="301"/>
        <source>OpenCascade settings file doesn&apos;t exist or is not readable [path=%1]</source>
        <translation>OpenCascade 设置文件不存在或不可读 [路径=%1]</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="181"/>
        <location filename="../src/cli/main.cpp" line="308"/>
        <source>OpenCascade settings file could not be loaded with QSettings [path=%1]</source>
        <translation>无法使用 QSettings 加载 OpenCascade 设置文件 [路径=%1]</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="339"/>
        <location filename="../src/cli/main.cpp" line="402"/>
        <source>Failed to load translation file [path=%1]</source>
        <translation>加载翻译文件失败 [路径=%1]</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="436"/>
        <source>[%1] %2</source>
        <translation>[%1] %2</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="439"/>
        <source>Unknown exception</source>
        <translation>未知异常</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="106"/>
        <source>Settings file(INI format) to load at startup</source>
        <translation>启动时加载的设置文件（INI 格式）</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="92"/>
        <source>Mayo the opensource 3D CAD viewer and converter</source>
        <translation>Mayo — 开源 3D CAD 查看器和转换器</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="107"/>
        <location filename="../src/app/main.cpp" line="114"/>
        <location filename="../src/cli/main.cpp" line="202"/>
        <location filename="../src/cli/main.cpp" line="215"/>
        <location filename="../src/cli/main.cpp" line="223"/>
        <location filename="../src/cli/main.cpp" line="230"/>
        <source>filepath</source>
        <translation>文件路径</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="183"/>
        <source>mayo-conv the opensource CAD converter</source>
        <translation>mayo-conv — 开源 CAD 转换器</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="188"/>
        <source>Display help on commandline options</source>
        <translation>显示命令行选项帮助</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="194"/>
        <source>Display version information</source>
        <translation>显示版本信息</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="200"/>
        <source>Use settings file(INI format) for the conversion. When this option isn&apos;t specified then cached settings are used</source>
        <translation>使用设置文件（INI 格式）进行转换。若未指定此选项，则使用缓存的设置</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="208"/>
        <source>Cache settings file provided with --use-settings for further use</source>
        <translation>缓存通过 --use-settings 提供的设置文件以备后用</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="214"/>
        <source>Write settings cache to an output file(INI format)</source>
        <translation>将设置缓存写入输出文件（INI 格式）</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="221"/>
        <source>Export opened files into an output file, can be repeated for different formats(eg. -e file.stp -e file.igs...)</source>
        <translation>将已打开的文件导出到输出文件，可对不同格式重复使用（例如 -e file.stp -e file.igs...）</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="242"/>
        <source>Disable progress reporting in console output</source>
        <translation>禁用控制台输出中的进度报告</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="254"/>
        <source>Files to open(import)</source>
        <translation>要打开（导入）的文件</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="453"/>
        <source>Error when writing to &apos;%1&apos;</source>
        <translation>写入&quot;%1&quot;时出错</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="455"/>
        <source>Settings cache written to %1</source>
        <translation>设置缓存已写入 %1</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="486"/>
        <source>Settings &apos;%1&apos; cached</source>
        <translation>设置&quot;%1&quot;已缓存</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="489"/>
        <source>No supplied settings to cache</source>
        <translation>没有提供要缓存的设置</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="307"/>
        <location filename="../src/cli/main.cpp" line="378"/>
        <source>Failed to load application settings file [path=%1]</source>
        <translation>加载应用程序设置文件失败 [路径=%1]</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="468"/>
        <source>No input files -&gt; nothing to export</source>
        <translation>没有输入文件 → 无内容可导出</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="395"/>
        <source>Failed to load theme &apos;%1&apos;</source>
        <translation>加载主题&quot;%1&quot;失败</translation>
    </message>
</context>
<context>
    <name>Mayo::MainWindow</name>
    <message>
        <location filename="../src/app/mainwindow.ui" line="14"/>
        <source>Mayo</source>
        <translation>Mayo</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.ui" line="53"/>
        <source>&amp;File</source>
        <translation>文件(&amp;F)</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.ui" line="58"/>
        <source>&amp;Help</source>
        <translation>帮助(&amp;H)</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.ui" line="63"/>
        <source>&amp;Tools</source>
        <translation>工具(&amp;T)</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.ui" line="68"/>
        <source>&amp;Window</source>
        <translation>窗口(&amp;W)</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.ui" line="73"/>
        <source>&amp;Display</source>
        <translation>显示(&amp;D)</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.cpp" line="279"/>
        <source>Information</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.cpp" line="283"/>
        <source>Warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.cpp" line="291"/>
        <source>Question</source>
        <translation>询问</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.cpp" line="221"/>
        <location filename="../src/app/mainwindow.cpp" line="287"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
</context>
<context>
    <name>Mayo::MeasureDisplayI18N</name>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="104"/>
        <source>Sum</source>
        <translation>总和</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="113"/>
        <source>(&lt;font color=&quot;#FF5500&quot;&gt;X&lt;/font&gt;{0} &lt;font color=&quot;#55FF00&quot;&gt;Y&lt;/font&gt;{1} &lt;font color=&quot;#0077FF&quot;&gt;Z&lt;/font&gt;{2}){3}</source>
        <translation>(&lt;font color=&quot;#FF5500&quot;&gt;X&lt;/font&gt;{0} &lt;font color=&quot;#55FF00&quot;&gt;Y&lt;/font&gt;{1} &lt;font color=&quot;#0077FF&quot;&gt;Z&lt;/font&gt;{2}){3}</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="130"/>
        <source> X{0} Y{1} Z{2}</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="254"/>
        <source>Diameter: {0}{1}</source>
        <translation>直径：{0}{1}</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="257"/>
        <source> Ø{0}</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="313"/>
        <location filename="../src/measure/measure_display.cpp" line="386"/>
        <location filename="../src/measure/measure_display.cpp" line="436"/>
        <location filename="../src/measure/measure_display.cpp" line="473"/>
        <source>{0}: {1}{2}</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="318"/>
        <source>{0}: {1}{2}&lt;br&gt;Point1: {3}&lt;br&gt;Point2: {4}</source>
        <translation>{0}：{1}{2}&lt;br&gt;点1：{3}&lt;br&gt;点2：{4}</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="387"/>
        <source>Angle</source>
        <translation>角度</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="437"/>
        <source>Length</source>
        <translation>长度</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="474"/>
        <source>Area</source>
        <translation>面积</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="530"/>
        <source>Min point: {0}&lt;br&gt;Max point: {1}&lt;br&gt;Size: {2} x {3} x {4}{5}&lt;br&gt;Volume: {6}{7}</source>
        <translation>最小点：{0}&lt;br&gt;最大点：{1}&lt;br&gt;尺寸：{2} × {3} × {4}{5}&lt;br&gt;体积：{6}{7}</translation>
    </message>
</context>
<context>
    <name>Mayo::Mesh_DocumentTreeNodeProperties</name>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="389"/>
        <source>NodeCount</source>
        <translation>节点数量</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="390"/>
        <source>TriangleCount</source>
        <translation>三角形数量</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="391"/>
        <source>Area</source>
        <translation>面积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="392"/>
        <source>Volume</source>
        <translation>体积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="411"/>
        <source>Data</source>
        <translation>数据</translation>
    </message>
</context>
<context>
    <name>Mayo::PointCloud_DocumentTreeNodeProperties</name>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="442"/>
        <source>PointCount</source>
        <translation>点数量</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="443"/>
        <source>HasColors</source>
        <translation>有颜色</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="444"/>
        <source>CornerMin</source>
        <translation>最小角点</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="445"/>
        <source>CornerMax</source>
        <translation>最大角点</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="464"/>
        <source>Data</source>
        <translation>数据</translation>
    </message>
</context>
<context>
    <name>Mayo::PropertyEditorI18N</name>
    <message>
        <location filename="../src/app/property_editor_factory.cpp" line="193"/>
        <source>Choose color ...</source>
        <translation>选择颜色...</translation>
    </message>
</context>
<context>
    <name>Mayo::PropertyItemDelegate</name>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="75"/>
        <source>%1d </source>
        <translation>%1天 </translation>
    </message>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="78"/>
        <source>%1h </source>
        <translation>%1时 </translation>
    </message>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="81"/>
        <source>%1min </source>
        <translation>%1分 </translation>
    </message>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="84"/>
        <source>%1s</source>
        <translation>%1秒</translation>
    </message>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="144"/>
        <location filename="../src/app/property_item_delegate.cpp" line="155"/>
        <source>%1%2</source>
        <translation>%1%2</translation>
    </message>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="287"/>
        <source>ERROR no stringifier for property type &apos;%1&apos;</source>
        <translation>错误：属性类型&quot;%1&quot;没有字符串化器</translation>
    </message>
</context>
<context>
    <name>Mayo::QStringUtils</name>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="45"/>
        <location filename="../src/app/qstring_utils.cpp" line="65"/>
        <source>(%1 %2 %3)</source>
        <translation>(%1 %2 %3)</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="79"/>
        <source>[%1; %2%3; %4]</source>
        <translation>[%1; %2%3; %4]</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="100"/>
        <source>B</source>
        <translation>字节</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="100"/>
        <location filename="../src/app/qstring_utils.cpp" line="102"/>
        <location filename="../src/app/qstring_utils.cpp" line="104"/>
        <location filename="../src/app/qstring_utils.cpp" line="106"/>
        <location filename="../src/app/qstring_utils.cpp" line="108"/>
        <source>%1%2</source>
        <translation>%1%2</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="102"/>
        <source>KB</source>
        <translation>KB</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="104"/>
        <location filename="../src/app/qstring_utils.cpp" line="106"/>
        <location filename="../src/app/qstring_utils.cpp" line="108"/>
        <source>MB</source>
        <translation>MB</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="113"/>
        <location filename="../src/app/qstring_utils.cpp" line="121"/>
        <source>Yes</source>
        <translation>是</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="113"/>
        <location filename="../src/app/qstring_utils.cpp" line="119"/>
        <source>No</source>
        <translation>否</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="120"/>
        <source>Partially</source>
        <translation>部分</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetClipPlanes</name>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="14"/>
        <source>Edit clip planes</source>
        <translation>编辑剖切平面</translation>
    </message>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="35"/>
        <source>X plane</source>
        <translation>X 平面</translation>
    </message>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="77"/>
        <location filename="../src/app/widget_clip_planes.ui" line="132"/>
        <location filename="../src/app/widget_clip_planes.ui" line="187"/>
        <location filename="../src/app/widget_clip_planes.ui" line="225"/>
        <source>Reverse plane</source>
        <translation>反转平面</translation>
    </message>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="80"/>
        <location filename="../src/app/widget_clip_planes.ui" line="135"/>
        <location filename="../src/app/widget_clip_planes.ui" line="190"/>
        <location filename="../src/app/widget_clip_planes.ui" line="228"/>
        <source>+/-</source>
        <translation>+/-</translation>
    </message>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="90"/>
        <source>Y plane</source>
        <translation>Y 平面</translation>
    </message>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="145"/>
        <source>Z plane</source>
        <translation>Z 平面</translation>
    </message>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="200"/>
        <source>Custom</source>
        <translation>自定义</translation>
    </message>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="270"/>
        <source>X </source>
        <translation>X </translation>
    </message>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="289"/>
        <source>Y </source>
        <translation>Y </translation>
    </message>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="308"/>
        <source>Z </source>
        <translation>Z </translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetExplodeAssembly</name>
    <message>
        <location filename="../src/app/widget_explode_assembly.ui" line="42"/>
        <source>%</source>
        <translation>%</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetFileSystem</name>
    <message>
        <location filename="../src/app/widget_file_system.cpp" line="104"/>
        <source>%1
Size: %2
Last modified: %3</source>
        <translation>%1
尺寸：%2
最后修改时间：%3</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetGrid</name>
    <message>
        <location filename="../src/app/widget_grid.ui" line="34"/>
        <source>Show Grid</source>
        <translation>显示网格</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="48"/>
        <source>Plane: XOY</source>
        <translation>平面： XOY</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="53"/>
        <source>Plane: ZOX</source>
        <translation>平面： ZOX</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="58"/>
        <source>Plane: YOZ</source>
        <translation>平面： YOZ</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="63"/>
        <source>Plane: Custom</source>
        <translation>平面： 自定义</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="95"/>
        <source>Configuration</source>
        <translation>配置</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="147"/>
        <source>Type</source>
        <translation>类型</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="164"/>
        <source>Rectangular</source>
        <translation>矩形的</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="169"/>
        <source>Circular</source>
        <translation>圆形</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="205"/>
        <location filename="../src/app/widget_grid.ui" line="244"/>
        <location filename="../src/app/widget_grid.ui" line="283"/>
        <location filename="../src/app/widget_grid.ui" line="426"/>
        <source>Y</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="212"/>
        <source>Step</source>
        <translation>步长</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="228"/>
        <location filename="../src/app/widget_grid.ui" line="270"/>
        <location filename="../src/app/widget_grid.ui" line="361"/>
        <location filename="../src/app/widget_grid.ui" line="413"/>
        <source>X</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="254"/>
        <source>Size</source>
        <translation>尺寸</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="290"/>
        <location filename="../src/app/widget_grid.ui" line="503"/>
        <source>Rotation</source>
        <translation>旋转角度</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="306"/>
        <location filename="../src/app/widget_grid.ui" line="519"/>
        <source>°</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="332"/>
        <location filename="../src/app/widget_grid.ui" line="535"/>
        <source>Offset</source>
        <translation>偏移量</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="368"/>
        <location filename="../src/app/widget_grid.ui" line="397"/>
        <source>Origin</source>
        <translation>起点</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="433"/>
        <source>Radius</source>
        <translation>圆半径</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="463"/>
        <source>Radius Step</source>
        <translation>半径步长</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="483"/>
        <source>Division</source>
        <translation>分割</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="574"/>
        <source>Graphics</source>
        <translation>图形</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="626"/>
        <location filename="../src/app/widget_grid.ui" line="643"/>
        <source>...</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="633"/>
        <source>Tenth Color</source>
        <translation>第十种颜色</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="650"/>
        <source>Mode</source>
        <translation>模式</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="661"/>
        <source>Lines</source>
        <translation>线条</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="666"/>
        <source>Points</source>
        <translation>点</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="674"/>
        <source>Color</source>
        <translation>颜色</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetGuiDocument</name>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="92"/>
        <source>Fit All</source>
        <translation>适合所有</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="94"/>
        <source>Edit Grid</source>
        <translation>编辑网格</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="97"/>
        <source>Edit clip planes</source>
        <translation>编辑剖切平面</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="100"/>
        <source>Explode assemblies</source>
        <translation>爆炸装配体</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="103"/>
        <source>Measure shapes</source>
        <translation>测量形状</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="342"/>
        <source>Isometric</source>
        <translation>等轴测</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="343"/>
        <source>Back</source>
        <translation>后视</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="344"/>
        <source>Front</source>
        <translation>前视</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="345"/>
        <source>Left</source>
        <translation>左视</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="346"/>
        <source>Right</source>
        <translation>右视</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="347"/>
        <source>Top</source>
        <translation>俯视</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="348"/>
        <source>Bottom</source>
        <translation>仰视</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="352"/>
        <source>&lt;b&gt;Left-click&lt;/b&gt;: popup menu of pre-defined views
&lt;b&gt;CTRL+Left-click&lt;/b&gt;: apply &apos;%1&apos; view</source>
        <translation>&lt;b&gt;左键单击&lt;/b&gt;：预定义视图弹出菜单&lt;br&gt;
&lt;b&gt;CTRL+左键单击&lt;/b&gt;：应用“%1”视图</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="387"/>
        <source>Show/hide items</source>
        <translation>显示/隐藏项目</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="395"/>
        <source>Show all</source>
        <translation>显示全部</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="396"/>
        <source>Show selection</source>
        <translation>显示所选</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="397"/>
        <source>Hide selection</source>
        <translation>隐藏所选</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="398"/>
        <source>Show only selection</source>
        <translation>仅显示所选</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetHomeFiles</name>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="44"/>
        <source>New Document</source>
        <translation>新建文档</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="56"/>
        <source>Open Document(s)</source>
        <translation>打开文档</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="125"/>
        <source>today %1</source>
        <translation>今天 %1</translation>
    </message>
    <message>
        <source>yersterday %1</source>
        <translation type="vanished">昨天 %1</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="135"/>
        <source>%1 days ago %2</source>
        <translation>%1 天前 %2</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="45"/>
        <source>

Create and add an empty document where you can import files</source>
        <translation>

创建并添加一个空白文档，用于导入文件</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="57"/>
        <source>

选择要加载的文件，并将其作为独立文档打开</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="128"/>
        <source>yesterday %1</source>
        <translation>昨天 %1</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="132"/>
        <location filename="../src/app/widget_home_files.cpp" line="139"/>
        <source>%1 %2</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="149"/>
        <source>%1

Size: %2

Created: %3
Modified: %4
Read: %5
</source>
        <translation>%1

大小：%2

创建时间：%3
修改时间：%4
访问时间：%5
</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetMainControl</name>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="106"/>
        <source>Model tree</source>
        <translation>模型树</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="111"/>
        <source>Opened documents</source>
        <translation>已打开的文档</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="116"/>
        <source>File system</source>
        <translation>文件系统</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="134"/>
        <source>Close Left Side Bar</source>
        <translation>关闭左侧栏</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="392"/>
        <source>X=</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="399"/>
        <location filename="../src/app/widget_main_control.ui" line="420"/>
        <location filename="../src/app/widget_main_control.ui" line="441"/>
        <source>?</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="413"/>
        <source>Y=</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="434"/>
        <source>Z=</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.cpp" line="382"/>
        <source>Data</source>
        <translation>数据</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.cpp" line="363"/>
        <source>Graphics</source>
        <translation>图形</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.cpp" line="343"/>
        <source>%1(%2)</source>
        <translation>%1(%2)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.cpp" line="412"/>
        <source>Options</source>
        <translation>选项</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.cpp" line="461"/>
        <source>Document file `%1` has been changed since it was opened

Do you want to reload that document?

File: `%2`</source>
        <translation>文档文件 `%1` 自打开以来已被修改

是否要重新加载该文档？

文件：`%2`</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.cpp" line="468"/>
        <source>Question</source>
        <translation>询问</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetMeasure</name>
    <message>
        <location filename="../src/app/widget_measure.ui" line="32"/>
        <source>Area Unit</source>
        <translation>面积单位</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="40"/>
        <source>Square Millimeter(mm²)</source>
        <translation>平方毫米 (mm²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="45"/>
        <source>Square Centimeter(cm²)</source>
        <translation>平方厘米 (cm²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="50"/>
        <source>Square Meter(m²)</source>
        <translation>平方米 (m²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="55"/>
        <source>Square Inch(in²)</source>
        <translation>平方英寸 (in²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="60"/>
        <source>Square Foot(ft²)</source>
        <translation>平方英尺 (ft²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="65"/>
        <source>Square Yard(yd²)</source>
        <translation>平方码 (yd²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="80"/>
        <source>Measure</source>
        <translation>测量</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="88"/>
        <source>Millimeter(mm)</source>
        <translation>毫米 (mm)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="93"/>
        <source>Centimeter(cm)</source>
        <translation>厘米 (cm)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="98"/>
        <source>Meter(m)</source>
        <translation>米 (m)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="103"/>
        <source>Inch(in)</source>
        <translation>英寸 (in)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="108"/>
        <source>Foot(ft)</source>
        <translation>英尺 (ft)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="113"/>
        <source>Yard(yd)</source>
        <translation>英尺 (ft)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="122"/>
        <source>Degree(°)</source>
        <translation>度 (°)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="127"/>
        <source>Radian(rad)</source>
        <translation>弧度 (rad)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="171"/>
        <source>Vertex Position</source>
        <translation>顶点位置</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="176"/>
        <source>Circle Center</source>
        <translation>圆心</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="181"/>
        <source>Circle Diameter</source>
        <translation>圆直径</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="186"/>
        <source>Min Distance</source>
        <translation>最小距离</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="191"/>
        <source>Center-to-center Distance</source>
        <translation>中心到中心距离</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="201"/>
        <source>Length</source>
        <translation>长度</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="211"/>
        <source>Bounding Box</source>
        <translation>包围盒</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="220"/>
        <source>Cubic Millimeter(mm³)</source>
        <translation>立方毫米 (mm³)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="225"/>
        <source>Cubic Centimeter(cm³)</source>
        <translation>立方厘米 (cm³)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="230"/>
        <source>Cubic Meter(m³)</source>
        <translation>立方米 (m³)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="235"/>
        <source>Cubic Inch(in³)</source>
        <translation>立方英寸 (in³)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="240"/>
        <source>Cubic Foot(ft³)</source>
        <translation>立方英尺 (ft³)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="245"/>
        <source>Liter(L)</source>
        <translation>升 (L)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="250"/>
        <source>Imperial Gallon(GBgal)</source>
        <translation>英制加仑 (GBgal)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="255"/>
        <source>US Gallon(USgal)</source>
        <translation>美制加仑 (USgal)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="196"/>
        <source>Angle</source>
        <translation>角度</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="206"/>
        <source>Surface Area</source>
        <translation>表面积</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="73"/>
        <source>Length Unit</source>
        <translation>长度单位</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="135"/>
        <source>Angle Unit</source>
        <translation>角度单位</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="163"/>
        <source>Volume Unit</source>
        <translation>体积单位</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.cpp" line="424"/>
        <source>Select entities to measure</source>
        <translation>选择要测量的实体</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetModelTree</name>
    <message>
        <location filename="../src/app/widget_model_tree.cpp" line="152"/>
        <source>Remove from document</source>
        <translation>从文档中删除</translation>
    </message>
    <message>
        <location filename="../src/app/widget_model_tree_builder.cpp" line="64"/>
        <source>&lt;unnamed&gt;</source>
        <translation>&lt;未命名&gt;</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetModelTreeBuilder_Xde</name>
    <message>
        <location filename="../src/app/widget_model_tree_builder_xde.cpp" line="70"/>
        <source>instanceNameFormat</source>
        <translation>装配实例名称格式</translation>
    </message>
    <message>
        <location filename="../src/app/widget_model_tree_builder_xde.cpp" line="128"/>
        <source>Show {}</source>
        <translation>显示 {}</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="31"/>
        <source>Instance</source>
        <translation>实例</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="32"/>
        <source>Product</source>
        <translation>产品</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="33"/>
        <source>Both</source>
        <translation>两者</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetPropertiesEditor</name>
    <message>
        <location filename="../src/app/widget_properties_editor.ui" line="58"/>
        <source>Property</source>
        <translation>属性</translation>
    </message>
    <message>
        <location filename="../src/app/widget_properties_editor.ui" line="63"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
</context>
<context>
    <name>Mayo::XCaf_DocumentTreeNodeProperties</name>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="58"/>
        <source>Name</source>
        <translation>名称</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="59"/>
        <source>Shape</source>
        <translation>形状</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="60"/>
        <source>XdeShape</source>
        <translation>XDE 形状类型</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="61"/>
        <source>XdeLayer</source>
        <translation>图层</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="62"/>
        <source>Color</source>
        <translation>颜色</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="63"/>
        <source>Location</source>
        <translation>位置</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="64"/>
        <source>Centroid</source>
        <translation>质心</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="65"/>
        <source>Area</source>
        <translation>面积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="66"/>
        <source>Volume</source>
        <translation>体积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="67"/>
        <source>MaterialDensity</source>
        <translation>材料密度</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="68"/>
        <source>MaterialName</source>
        <translation>材料名称</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="70"/>
        <source>ProductName</source>
        <translation>产品名称</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="71"/>
        <source>ProductColor</source>
        <translation>产品颜色</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="72"/>
        <source>ProductCentroid</source>
        <translation>产品质心</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="73"/>
        <source>ProductArea</source>
        <translation>产品面积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="74"/>
        <source>ProductVolume</source>
        <translation>产品体积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="302"/>
        <source>Assembly</source>
        <translation>装配体</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="305"/>
        <source>Reference</source>
        <translation>实例</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="308"/>
        <source>Component</source>
        <translation>组件</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="311"/>
        <source>Compound</source>
        <translation>复合体</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="314"/>
        <source>Simple</source>
        <translation>简单形状</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="317"/>
        <source>Sub</source>
        <translation>子形状</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="359"/>
        <source>Data</source>
        <translation>数据</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="360"/>
        <source>Validation</source>
        <translation>验证</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="361"/>
        <source>MetaData</source>
        <translation>元数据</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="362"/>
        <source>ProductMetaData</source>
        <translation>产品元数据</translation>
    </message>
</context>
<context>
    <name>OpenCascade::Aspect_HatchStyle</name>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="45"/>
        <source>Solid</source>
        <translation>实线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="46"/>
        <source>Horizontal</source>
        <translation>水平线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="47"/>
        <source>HorizontalSparse</source>
        <translation>稀疏水平线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="48"/>
        <source>Vertical</source>
        <translation>垂直线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="49"/>
        <source>VerticalSparse</source>
        <translation>稀疏垂直线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="50"/>
        <source>Diagonal45</source>
        <translation>45° 斜线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="51"/>
        <source>Diagonal45Sparse</source>
        <translation>稀疏 45° 斜线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="52"/>
        <source>Diagonal135</source>
        <translation>135° 斜线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="53"/>
        <source>Diagonal135Sparse</source>
        <translation>稀疏 135° 斜线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="54"/>
        <source>Grid</source>
        <translation>网格</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="55"/>
        <source>GridSparse</source>
        <translation>稀疏网格</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="56"/>
        <source>GridDiagonal</source>
        <translation>对角网格</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="57"/>
        <source>GridDiagonalSparse</source>
        <translation>稀疏对角网格</translation>
    </message>
</context>
<context>
    <name>OpenCascade::Graphic3d_NameOfMaterial</name>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="18"/>
        <source>Brass</source>
        <translation>黄铜</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="19"/>
        <source>Bronze</source>
        <translation>青铜</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="20"/>
        <source>Copper</source>
        <translation>铜</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="21"/>
        <source>Gold</source>
        <translation>金</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="22"/>
        <source>Pewter</source>
        <translation>锡镴</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="23"/>
        <source>Plaster</source>
        <translation>石膏</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="24"/>
        <source>Plastic</source>
        <translation>塑料</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="25"/>
        <source>Silver</source>
        <translation>银</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="26"/>
        <source>Steel</source>
        <translation>钢</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="27"/>
        <source>Stone</source>
        <translation>石材</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="28"/>
        <source>ShinyPlastic</source>
        <translation>光亮塑料</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="29"/>
        <source>Satin</source>
        <translation>缎面</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="30"/>
        <source>Metalized</source>
        <translation>金属化</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="31"/>
        <source>NeonGnc</source>
        <translation>霓虹（GNC）</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="32"/>
        <source>Chrome</source>
        <translation>铬</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="33"/>
        <source>Aluminium</source>
        <translation>铝</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="34"/>
        <source>Obsidian</source>
        <translation>黑曜石</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="35"/>
        <source>NeonPhc</source>
        <translation>霓虹（PHC）</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="36"/>
        <source>Jade</source>
        <translation>玉石</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="37"/>
        <source>Default</source>
        <translation>默认</translation>
    </message>
</context>
</TS>

<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>Mayo::AppModule</name>
    <message>
        <location filename="../src/app/app_module.cpp" line="124"/>
        <source>en</source>
        <translation>英文</translation>
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
        <location filename="messages.cpp" line="17"/>
        <source>SI</source>
        <translation>国际单位制</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="18"/>
        <source>ImperialUK</source>
        <translation>英制单位</translation>
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
        <translation>标准</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="23"/>
        <source>Precise</source>
        <translation>精确</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="24"/>
        <source>VeryPrecise</source>
        <translation>高精度</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="25"/>
        <source>UserDefined</source>
        <translation>自定义</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="27"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="28"/>
        <source>ReloadIfUserConfirm</source>
        <translation>用户确认后重载</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="29"/>
        <source>ReloadSilently</source>
        <translation>静默重载</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="26"/>
        <source>TopLeft</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="27"/>
        <source>TopRight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="28"/>
        <source>BottomLeft</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="29"/>
        <source>BottomRight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="38"/>
        <source>system</source>
        <translation>系统</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="39"/>
        <source>application</source>
        <translation>应用程序</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="40"/>
        <source>meshing</source>
        <translation>网格划分</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="41"/>
        <source>graphics</source>
        <translation>图形</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="42"/>
        <source>language</source>
        <translation>界面语言</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="43"/>
        <source>viewCubeCorner</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="46"/>
        <source>units</source>
        <translation>单位</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="47"/>
        <source>clipPlanes</source>
        <translation>剖面</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="48"/>
        <source>meshDefaults</source>
        <translation>默认网格</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="147"/>
        <source>import</source>
        <translation>导入</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="164"/>
        <source>export</source>
        <translation>导出</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="188"/>
        <source>Language used for the application. Change will take effect after application restart</source>
        <translation>语言修改后需要重启后生效！（如果中文翻译有错误请在B站搜索[CP设计]向我反馈）</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="192"/>
        <source>Action to be done after some opened document file is changed(modified) externally

Select options `{0}` or `{1}` so the application monitors changes made to opened files

When such a change is detected then the application proposes to reload(open again) the document

Select `{1}` to automatically reload documents without any user interaction</source>
        <translation>外部修改已打开文档后的处理策略：

选择`{0}`或`{1}`选项可启用文件修改监控

检测到变更时将提示文档重载

选择`{1}`可自动静默重载文档</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="201"/>
        <source>In case where multiple documents are opened, make sure the document displayed in the 3D view corresponds to what is selected in the model tree</source>
        <translation>同时打开多文档时，确保3D视图显示内容与模型树选择项同步</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="205"/>
        <source>Force usage of the fallback Qt widget to display OpenGL graphics.

When `OFF` the application will try to use OpenGL framebuffer for rendering, this allows to display overlay widgets(eg measure tools panel) with translucid background. However using OpenGL framebuffer might cause troubles for some users(eg empty 3D window) especially on macOS.

When `ON` the application will use a regular Qt widget for rendering which proved to be more supported.

This option is applicable when OpenCascade ≥ 7.6 version. Change will take effect after application restart</source>
        <translation>强制使用Qt备用控件显示OpenGL图形：

关闭时：尝试使用OpenGL帧缓冲渲染，支持半透明背景覆盖控件（如测量工具面板）
但可能导致部分用户（特别是macOS）出现3D窗口空白问题

开启时：使用标准Qt控件渲染，兼容性更好

适用于OpenCascade ≥7.6版本，重启后生效</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="219"/>
        <source>Controls precision of the mesh to be computed from the BRep shape</source>
        <translation>控制BRep形状生成的网格精度</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="222"/>
        <source>For the tessellation of faces the chordal deflection limits the distance between a curve and its tessellation</source>
        <translation>弦高公差：限制曲线与网格折线间的最大距离</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="226"/>
        <source>For the tessellation of faces the angular deflection limits the angle between subsequent segments in a polyline</source>
        <translation>角度公差：限制折线相邻线段间的最大角度</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="230"/>
        <source>Relative computation of edge tolerance

If activated, deflection used for the polygonalisation of each edge will be `ChordalDeflection` &amp;#215; `SizeOfEdge`. The deflection used for the faces will be the maximum deflection of their edges.</source>
        <translation>边缘公差的相对计算

如果激活，每个边缘的多边形化使用的偏差将是 `ChordalDeflection` &amp;#215; `SizeOfEdge`。面使用的偏差将是其边缘的最大偏差。</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="238"/>
        <source>3D view manipulation shortcuts configuration to mimic other common CAD applications</source>
        <translation>配置3D视图操作快捷键以匹配主流CAD软件</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="241"/>
        <source>Angle increment used to turn(rotate) the 3D view around the normal of the view plane(Z axis frame reference)</source>
        <translation>视图绕Z轴旋转时的角度增量（度）</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="243"/>
        <source>Corner where 3D view cube is located</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="247"/>
        <source>Show or hide by default the trihedron centered at world origin. This doesn&apos;t affect 3D view of currently opened documents</source>
        <translation>默认显示世界坐标系（不影响已打开文档）</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="251"/>
        <source>Enable capping of currently clipped graphics</source>
        <translation>启用剖面封口显示</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.cpp" line="254"/>
        <source>Enable capping hatch texture of currently clipped graphics</source>
        <translation>启用剖面线纹理</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="57"/>
        <source>decimalCount</source>
        <translation>小数位数</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="58"/>
        <source>schema</source>
        <translation>协议模式</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="62"/>
        <source>recentFiles</source>
        <translation>最近文件</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="63"/>
        <source>lastOpenFolder</source>
        <translation>最后打开目录</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="64"/>
        <source>lastSelectedFormatFilter</source>
        <translation>最后选择的格式过滤器</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="65"/>
        <source>actionOnDocumentFileChange</source>
        <translation>文档外部修改响应</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="66"/>
        <source>linkWithDocumentSelector</source>
        <translation>与文档选择器联动</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="67"/>
        <source>forceOpenGlFallbackWidget</source>
        <translation>强制OpenGL备用控件</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="68"/>
        <source>appUiState</source>
        <translation>界面状态</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="72"/>
        <source>meshingQuality</source>
        <translation>网格质量</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="73"/>
        <source>meshingChordalDeflection</source>
        <translation>网格弦高公差</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="74"/>
        <source>meshingAngularDeflection</source>
        <translation>网格角度公差</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="75"/>
        <source>meshingRelative</source>
        <translation>相对网格计算</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="78"/>
        <source>navigationStyle</source>
        <translation>导航样式</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="80"/>
        <source>defaultShowOriginTrihedron</source>
        <translation>默认显示原点坐标轴</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="81"/>
        <source>instantZoomFactor</source>
        <translation>实时缩放系数</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="82"/>
        <source>turnViewAngleIncrement</source>
        <translation>视图旋转角度增量</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="84"/>
        <source>cappingOn</source>
        <translation>启用封口</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="85"/>
        <source>cappingHatchOn</source>
        <translation>启用剖面线封口</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="87"/>
        <source>color</source>
        <translation>颜色</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="88"/>
        <source>edgeColor</source>
        <translation>边线颜色</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="89"/>
        <source>material</source>
        <translation>材质</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="90"/>
        <source>showEgesOn</source>
        <translation>显示边线</translation>
    </message>
    <message>
        <location filename="../src/app/app_module_properties.h" line="91"/>
        <source>showNodesOn</source>
        <translation>显示节点</translation>
    </message>
</context>
<context>
    <name>Mayo::Application</name>
    <message>
        <location filename="../src/base/application.cpp" line="150"/>
        <source>Binary Mayo Document Format</source>
        <translation>Mayo二进制文档格式</translation>
    </message>
    <message>
        <location filename="../src/base/application.cpp" line="155"/>
        <source>XML Mayo Document Format</source>
        <translation>Mayo XML文档格式</translation>
    </message>
</context>
<context>
    <name>Mayo::BRepMeasureError</name>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="75"/>
        <source>Entity must be a vertex</source>
        <translation>实体必须是顶点</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="77"/>
        <source>Entity must be a circular edge</source>
        <translation>实体必须是圆形边线</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="79"/>
        <source>Entity must be a shape(BREP)</source>
        <translation>实体必须是BREP形状</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="81"/>
        <source>Entity must be a geometric or polygon edge</source>
        <translation>实体必须是几何或多边形边线</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="83"/>
        <source>Entity must be a geometric or triangulation face</source>
        <translation>实体必须是几何或三角化面</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="85"/>
        <source>Computation of minimum distance failed</source>
        <translation>最小距离计算失败</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="87"/>
        <source>Unable to find center of the shape</source>
        <translation>无法找到形状中心</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="89"/>
        <source>All entities must be edges</source>
        <translation>所有实体必须为边线</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="91"/>
        <source>Entity must be a linear edge</source>
        <translation>实体必须是直线边线</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="93"/>
        <source>All entities must be faces</source>
        <translation>所有实体必须为面</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="95"/>
        <source>Entities must not be parallel</source>
        <translation>实体不得平行</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="97"/>
        <source>Bounding box computed is void</source>
        <translation>计算的边界框为空</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_tool_brep.cpp" line="99"/>
        <source>Unknown error</source>
        <translation>未知错误</translation>
    </message>
</context>
<context>
    <name>Mayo::CliExport</name>
    <message>
        <location filename="../src/cli/cli_export.cpp" line="115"/>
        <source>Mesh BRep shapes</source>
        <translation>网格化BREP形状</translation>
    </message>
    <message>
        <location filename="../src/cli/cli_export.cpp" line="119"/>
        <source>Imported</source>
        <translation>已导入</translation>
    </message>
    <message>
        <location filename="../src/cli/cli_export.cpp" line="143"/>
        <source>Exported {}</source>
        <translation>已导出{}</translation>
    </message>
    <message>
        <location filename="../src/cli/cli_export.cpp" line="223"/>
        <source>Importing...</source>
        <translation>正在导入...</translation>
    </message>
    <message>
        <location filename="../src/cli/cli_export.cpp" line="235"/>
        <source>Exporting {}...</source>
        <translation>正在导出{}...</translation>
    </message>
</context>
<context>
    <name>Mayo::Command</name>
    <message>
        <location filename="../src/app/command_system_information.cpp" line="54"/>
        <source>System Information...</source>
        <translation>系统信息...</translation>
    </message>
    <message>
        <location filename="../src/app/command_system_information.cpp" line="73"/>
        <source>Copy to Clipboard</source>
        <translation>复制到剪贴板</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="41"/>
        <source>Orthographic</source>
        <translation>正交</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="42"/>
        <source>Perspective</source>
        <translation>透视</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="56"/>
        <source>Projection</source>
        <translation>投影模式</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="102"/>
        <source>Mode</source>
        <translation>模式</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="148"/>
        <source>[%1] %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="172"/>
        <source>Show Origin Trihedron</source>
        <translation>显示原点坐标轴</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="173"/>
        <source>Show/Hide Origin Trihedron</source>
        <translation>显示/隐藏原点坐标轴</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="210"/>
        <source>Show Performance Stats</source>
        <translation>显示性能统计</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="211"/>
        <source>Show/Hide rendering performance statistics</source>
        <translation>显示/隐藏渲染性能统计</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="248"/>
        <source>Zoom In</source>
        <translation>放大</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="265"/>
        <source>Zoom Out</source>
        <translation>缩小</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="282"/>
        <source>Turn Counter Clockwise</source>
        <translation>逆时针旋转</translation>
    </message>
    <message>
        <location filename="../src/app/commands_display.cpp" line="300"/>
        <source>Turn Clockwise</source>
        <translation>顺时针旋转</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="51"/>
        <source>%1 files(%2)</source>
        <extracomment>%1 is the format identifier and %2 is the file filters string</extracomment>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="113"/>
        <source>All files(*.*)</source>
        <translation>所有文件(*.*)</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="115"/>
        <source>Select Part File</source>
        <translation>选择零件文件</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="198"/>
        <location filename="../src/app/commands_file.cpp" line="241"/>
        <source>Mesh BRep shapes</source>
        <translation>网格化BREP形状</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="203"/>
        <location filename="../src/app/commands_file.cpp" line="246"/>
        <source>Import time: {}ms</source>
        <translation>导入耗时：{}毫秒</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="250"/>
        <location filename="../src/app/commands_file.cpp" line="383"/>
        <source>Import</source>
        <translation>导入</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="268"/>
        <source>New</source>
        <translation>新建</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="269"/>
        <source>New Document</source>
        <translation>新建文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="278"/>
        <source>Anonymous%1</source>
        <translation>未命名%1</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="285"/>
        <source>Open</source>
        <translation>打开</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="286"/>
        <source>Open Documents</source>
        <translation>打开文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="336"/>
        <source>Recent files</source>
        <translation>最近文件</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="362"/>
        <source>%1 | %2</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="370"/>
        <source>Clear menu</source>
        <translation>清空菜单</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="384"/>
        <source>Import in current document</source>
        <translation>导入到当前文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="414"/>
        <location filename="../src/app/commands_file.cpp" line="415"/>
        <source>Export selected items</source>
        <translation>导出选中项</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="424"/>
        <source>No item selected for export</source>
        <translation>没有选要导出的物体</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="436"/>
        <source>Select Output File</source>
        <translation>选择输出文件</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="459"/>
        <source>Export time: {}ms</source>
        <translation>导出耗时：{}毫秒</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="476"/>
        <source>Close &quot;%1&quot;</source>
        <translation>关闭&quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="510"/>
        <source>Close %1</source>
        <translation>关闭%1</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="511"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="521"/>
        <source>Close all</source>
        <translation>全部关闭</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="522"/>
        <source>Close all documents</source>
        <translation>关闭所有文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="542"/>
        <location filename="../src/app/commands_file.cpp" line="583"/>
        <source>Close all except current</source>
        <translation>关闭除当前外的所有</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="543"/>
        <source>Close all except current document</source>
        <translation>关闭除当前文档外的所有</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="582"/>
        <source>Close all except %1</source>
        <translation>关闭除%1外的所有</translation>
    </message>
    <message>
        <location filename="../src/app/commands_file.cpp" line="592"/>
        <source>Quit</source>
        <translation>退出</translation>
    </message>
    <message>
        <location filename="../src/app/commands_help.cpp" line="26"/>
        <source>Report Bug</source>
        <translation>报告错误</translation>
    </message>
    <message>
        <location filename="../src/app/commands_help.cpp" line="39"/>
        <source>About %1</source>
        <translation>关于%1</translation>
    </message>
    <message>
        <location filename="../src/app/commands_tools.cpp" line="28"/>
        <location filename="../src/app/commands_tools.cpp" line="29"/>
        <source>Save View to Image</source>
        <translation>保存视图为图像</translation>
    </message>
    <message>
        <location filename="../src/app/commands_tools.cpp" line="51"/>
        <location filename="../src/app/commands_tools.cpp" line="52"/>
        <source>Inspect XDE</source>
        <translation>检查XDE</translation>
    </message>
    <message>
        <location filename="../src/app/commands_tools.cpp" line="90"/>
        <location filename="../src/app/commands_tools.cpp" line="91"/>
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
        <location filename="../src/app/commands_window.cpp" line="49"/>
        <source>Show/Hide Left Sidebar</source>
        <translation>显示/隐藏左侧边栏</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="84"/>
        <source>Hide Left Sidebar</source>
        <translation>隐藏左侧边栏</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="88"/>
        <source>Show Left Sidebar</source>
        <translation>显示左侧边栏</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="99"/>
        <location filename="../src/app/commands_window.cpp" line="151"/>
        <source>Go To Home Page</source>
        <translation>前往主页</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="148"/>
        <source>Go To Documents</source>
        <translation>前往说明文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="162"/>
        <location filename="../src/app/commands_window.cpp" line="163"/>
        <source>Previous Document</source>
        <translation>上一个文档</translation>
    </message>
    <message>
        <location filename="../src/app/commands_window.cpp" line="188"/>
        <location filename="../src/app/commands_window.cpp" line="189"/>
        <source>Next Document</source>
        <translation>下一个文档</translation>
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
        <source>Mayo By Fougue Ltd.</source>
        <translation>Mayo By Fougue Ltd（汉化 By CP设计[B站]）.</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_about.ui" line="60"/>
        <source>Version %1 (%2bit)</source>
        <translation>版本%1 (%2位)</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_about.ui" line="67"/>
        <source>Built on %1 at %2</source>
        <translation>构建于%1 %2</translation>
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
        <location filename="../src/app/dialog_about.cpp" line="23"/>
        <source>%1 By %2</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/dialog_about.cpp" line="43"/>
        <source>%1 %2</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Mayo::DialogInspectXde</name>
    <message>
        <location filename="../src/app/dialog_inspect_xde.ui" line="14"/>
        <source>XDE</source>
        <translation>XDE检查器</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="129"/>
        <source>ShapeType=%1, ShapeLocation=%2, Evolution=%3</source>
        <translation>类型=%1，位置=%2，演变=%3</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="174"/>
        <source>Yes</source>
        <translation>是</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="175"/>
        <source>No</source>
        <translation>否</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="381"/>
        <source>File Size: %1&lt;br&gt;Dimensions: %2x%3 Depth: %4</source>
        <translation>文件大小：%1&lt;br&gt;尺寸：%2×%3 深度：%4</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="386"/>
        <source>Error when loading texture file(invalid path?)</source>
        <translation>加载纹理文件失败（路径无效？）</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="417"/>
        <source>%1,offset:%2</source>
        <translation>%1，偏移量：%2</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="433"/>
        <source>&lt;data&gt;</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="827"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="827"/>
        <source>This document is not suitable for XDE</source>
        <translation>当前文档不适用于XDE检查</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="849"/>
        <source>Attributes</source>
        <translation>属性</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="863"/>
        <source>Shape</source>
        <translation>形状</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="867"/>
        <source>Color</source>
        <translation>颜色</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="871"/>
        <source>Material</source>
        <translation>材质</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="876"/>
        <source>VisMaterial</source>
        <translation>可视化材质</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="881"/>
        <source>Dimension</source>
        <translation>尺寸</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="884"/>
        <source>Datum</source>
        <translation>基准</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_inspect_xde.cpp" line="887"/>
        <source>GeomTolerance</source>
        <translation>几何公差</translation>
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
        <location filename="../src/app/dialog_options.cpp" line="151"/>
        <source>Restore default values</source>
        <translation>恢复默认值</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="186"/>
        <source>%1 / %2</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="270"/>
        <source>Exchange</source>
        <translation>交换</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="273"/>
        <source>Load from file...</source>
        <translation>从文件加载...</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="274"/>
        <source>Save as...</source>
        <translation>另存为...</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="355"/>
        <location filename="../src/app/dialog_options.cpp" line="378"/>
        <source>Choose INI file</source>
        <translation>选择INI文件</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="355"/>
        <location filename="../src/app/dialog_options.cpp" line="378"/>
        <source>INI files(*.ini)</source>
        <translation>INI文件(*.ini)</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="361"/>
        <location filename="../src/app/dialog_options.cpp" line="366"/>
        <location filename="../src/app/dialog_options.cpp" line="387"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="361"/>
        <source>&apos;%1&apos; doesn&apos;t exist</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="366"/>
        <source>&apos;%1&apos; is not readable</source>
        <translation>&apos;%1&apos; 不可读</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="387"/>
        <source>Error when writing to &apos;%1&apos;</source>
        <translation>写入&apos;%1&apos;时发生错误</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="413"/>
        <source>Restore values for default section only</source>
        <translation>仅恢复默认区域的值</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_options.cpp" line="416"/>
        <source>Restore values for the whole group</source>
        <translation>恢复整组数值</translation>
    </message>
</context>
<context>
    <name>Mayo::DialogSaveImageView</name>
    <message>
        <location filename="../src/app/dialog_save_image_view.ui" line="14"/>
        <source>Save View to Image</source>
        <translation>保存视图为图像</translation>
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
        <location filename="../src/app/dialog_save_image_view.cpp" line="120"/>
        <source>Keep ratio</source>
        <translation>保持比例</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="44"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="45"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="46"/>
        <source>Preview</source>
        <translation>预览</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="69"/>
        <source>%1 files(*.%2)</source>
        <translation>%1 文件(*.%2)</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="76"/>
        <source>Select image file</source>
        <translation>选择图像文件</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="95"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="95"/>
        <source>Failed to save image &apos;%1&apos;</source>
        <translation>保存图像&apos;%1&apos;失败</translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="117"/>
        <source>%1x%2 %3</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/dialog_save_image_view.cpp" line="120"/>
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
        <location filename="../src/app/dialog_task_manager.cpp" line="191"/>
        <source> / </source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Mayo::DocumentPropertyGroup</name>
    <message>
        <location filename="../src/app/document_property_group.h" line="21"/>
        <source>filepath</source>
        <translation>文件路径</translation>
    </message>
    <message>
        <location filename="../src/app/document_property_group.h" line="22"/>
        <source>fileSize</source>
        <translation>文件大小</translation>
    </message>
    <message>
        <location filename="../src/app/document_property_group.h" line="23"/>
        <source>createdDateTime</source>
        <translation>创建时间</translation>
    </message>
    <message>
        <location filename="../src/app/document_property_group.h" line="24"/>
        <source>modifiedDateTime</source>
        <translation>修改时间</translation>
    </message>
    <message>
        <location filename="../src/app/document_property_group.h" line="25"/>
        <source>owner</source>
        <translation>所有者</translation>
    </message>
    <message>
        <location filename="../src/app/document_property_group.h" line="26"/>
        <source>entityCount</source>
        <translation>实体数量</translation>
    </message>
</context>
<context>
    <name>Mayo::GraphicsMeshObjectDriver</name>
    <message>
        <source>Mesh_Wireframe</source>
        <translation type="vanished">线框网格</translation>
    </message>
    <message>
        <source>Mesh_Shaded</source>
        <translation type="vanished">着色网格</translation>
    </message>
    <message>
        <source>Mesh_Shrink</source>
        <translation type="vanished">收缩网格</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="37"/>
        <source>Wireframe</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="38"/>
        <source>Shaded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="39"/>
        <source>Shrink</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="201"/>
        <source>color</source>
        <translation>颜色</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="202"/>
        <source>edgeColor</source>
        <translation>边线颜色</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="203"/>
        <source>showEdges</source>
        <translation>显示边线</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_mesh_object_driver.cpp" line="204"/>
        <source>showNodes</source>
        <translation>显示节点</translation>
    </message>
</context>
<context>
    <name>Mayo::GraphicsMeshObjectDriverI18N</name>
    <message>
        <source>Mesh_Wireframe</source>
        <translation type="obsolete">线框网格</translation>
    </message>
    <message>
        <source>Mesh_Shaded</source>
        <translation type="obsolete">着色网格</translation>
    </message>
    <message>
        <source>Mesh_Shrink</source>
        <translation type="obsolete">收缩网格</translation>
    </message>
    <message>
        <source>color</source>
        <translation type="obsolete">颜色</translation>
    </message>
    <message>
        <source>edgeColor</source>
        <translation type="obsolete">边线颜色</translation>
    </message>
    <message>
        <source>showEdges</source>
        <translation type="obsolete">显示边线</translation>
    </message>
    <message>
        <source>showNodes</source>
        <translation type="obsolete">显示节点</translation>
    </message>
</context>
<context>
    <name>Mayo::GraphicsObjectDriverI18N</name>
    <message>
        <location filename="messages.cpp" line="36"/>
        <source>GraphicsShapeObjectDriver</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="messages.cpp" line="37"/>
        <source>GraphicsMeshObjectDriver</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="messages.cpp" line="38"/>
        <source>GraphicsPointCloudObjectDriver</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Mayo::GraphicsShapeObjectDriver</name>
    <message>
        <source>Shape_Wireframe</source>
        <translation type="vanished">线框模式</translation>
    </message>
    <message>
        <source>Shape_HiddenLineRemoval</source>
        <translation type="vanished">工程图模式</translation>
    </message>
    <message>
        <source>Shape_Shaded</source>
        <translation type="vanished">着色模式</translation>
    </message>
    <message>
        <source>Shape_ShadedWithFaceBoundary</source>
        <translation type="vanished">着色模式(显示曲面边缘)</translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_shape_object_driver.cpp" line="33"/>
        <source>Wireframe</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_shape_object_driver.cpp" line="34"/>
        <source>HiddenLineRemoval</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_shape_object_driver.cpp" line="35"/>
        <source>Shaded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/graphics/graphics_shape_object_driver.cpp" line="36"/>
        <source>ShadedWithFaceBoundary</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Mayo::IO::AssimpReaderI18N</name>
    <message>
        <location filename="../src/io_assimp/io_assimp_reader.cpp" line="369"/>
        <source>LINE primitives not supported yet</source>
        <translation>暂不支持LINE图元</translation>
    </message>
    <message>
        <location filename="../src/io_assimp/io_assimp_reader.cpp" line="372"/>
        <source>Some primitive not supported</source>
        <translation>部分图元不受支持</translation>
    </message>
    <message>
        <location filename="../src/io_assimp/io_assimp_reader.cpp" line="503"/>
        <source>Texture not found: {}
Tried:</source>
        <translation>未找到纹理：{} 已尝试：</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::DxfReader::Properties</name>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="155"/>
        <source>Scale entities according some factor</source>
        <translation>根据系数缩放实体</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="157"/>
        <source>Import text/dimension objects</source>
        <translation>导入文本/标注对象</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="159"/>
        <source>Group all objects within a layer into a single compound shape</source>
        <translation>将图层内所有对象打组为一个复合形状</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="161"/>
        <source>Name of the font to be used when creating shape for text objects</source>
        <translation>用于文本对象创建形状的字体名称</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="172"/>
        <source>scaling</source>
        <translation>缩放比例</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="173"/>
        <source>importAnnotations</source>
        <translation>导入注释</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="174"/>
        <source>groupLayers</source>
        <translation>群组图层</translation>
    </message>
    <message>
        <location filename="../src/io_dxf/io_dxf.cpp" line="175"/>
        <source>fontNameForTextObjects</source>
        <translation>文本对象字体</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::GmioAmfWriter::Properties</name>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="133"/>
        <source>Format used when writing `double` values as strings</source>
        <translation>写入双精度值时使用的字符串格式</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="135"/>
        <source>Decimal floating point(ex: 392.65)</source>
        <translation>十进制浮点数（例如：392.65）</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="136"/>
        <source>Scientific notation(ex: 3.9265E+2)</source>
        <translation>科学记数法（例如：3.9265E+2）</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="137"/>
        <source>Use the shortest representation: decimal or scientific</source>
        <translation>使用最短表示法：十进制或科学记数法</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="143"/>
        <source>Maximum number of significant digits when writing `double` values</source>
        <translation>写入双精度值时的最大有效位数</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="146"/>
        <source>Write AMF document in ZIP archive containing one file entry</source>
        <translation>将AMF文档写入包含单个文件的ZIP存档</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="149"/>
        <source>Filename of the single AMF entry within the ZIP archive.
Only applicable if option `{}` is on</source>
        <translation>ZIP存档中单个AMF条目的文件名。
仅在选项 `{}` 开启时适用</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="154"/>
        <source>Use the ZIP64 format extensions.
Only applicable if option `{}` is on</source>
        <translation>使用ZIP64格式扩展。
仅在选项 `{}` 开启时适用</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="182"/>
        <source>float64Format</source>
        <translation>双精度格式</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="183"/>
        <source>float64Precision</source>
        <translation>双精度位数</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="184"/>
        <source>createZipArchive</source>
        <translation>创建ZIP存档</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="185"/>
        <source>zipEntryFilename</source>
        <translation>ZIP条目文件名</translation>
    </message>
    <message>
        <location filename="../src/io_gmio/io_gmio_amf_writer.cpp" line="186"/>
        <source>useZip64</source>
        <translation>使用ZIP64</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="41"/>
        <source>Decimal</source>
        <translation>十进制</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="42"/>
        <source>Scientific</source>
        <translation>科学记数法</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="43"/>
        <source>Shortest</source>
        <translation>最短</translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="62"/>
        <source>End color of the image background gradient</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="65"/>
        <source>Type of gradient fill for the image background</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="69"/>
        <source>No gadient fill, single color background</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="70"/>
        <source>Gradient directed from left to right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="71"/>
        <source>Gradient directed from top to bottom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="72"/>
        <source>Gradient directed from top left corner to bottom right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="73"/>
        <source>Gradient directed from top right corner to bottom left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="74"/>
        <source>Gradient directed from center in all directions forming concentric circles</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="78"/>
        <source>Camera orientation expressed in Z-up convention as a unit vector</source>
        <translation>以Z轴向上的约定表示的单位向量相机方向</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="81"/>
        <source>Camera projection type, specifies how the 3D scene is projected onto a 2D image for display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="101"/>
        <source>Graphics display mode for the objects of type `{}`</source>
        <translation type="unfinished"></translation>
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
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="131"/>
        <source>backgroundColorEnd</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="132"/>
        <source>backgroundGradientFill</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="200"/>
        <source>Background radial gradient fill is available since OpenCascade 7.6.
Default to background single color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>backgroundColor</source>
        <translation type="vanished">背景颜色</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="133"/>
        <source>cameraOrientation</source>
        <translation>相机方向</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="134"/>
        <source>cameraProjection</source>
        <translation>相机投影方式</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="188"/>
        <source>No transferred application items</source>
        <translation>没有传输的应用项目</translation>
    </message>
    <message>
        <location filename="../src/io_image/io_image.cpp" line="196"/>
        <source>Camera orientation vector must not be null</source>
        <translation>相机方向向量不可为空</translation>
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
        <translation type="unfinished">无</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="72"/>
        <source>Horizontal</source>
        <translation type="unfinished">水平线</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="73"/>
        <source>Vertical</source>
        <translation type="unfinished">垂直线</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="74"/>
        <source>DiagonalTopLeftBottomRight</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="messages.cpp" line="75"/>
        <source>DiagonalTopRightBottomLeft</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="messages.cpp" line="76"/>
        <source>Radial</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="messages.cpp" line="77"/>
        <source>GraphicsShapeObjectDriver_displayMode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="messages.cpp" line="78"/>
        <source>GraphicsMeshObjectDriver_displayMode</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccBaseMeshReaderProperties</name>
    <message>
        <location filename="../src/io_occ/io_occ_base_mesh.cpp" line="28"/>
        <source>rootPrefix</source>
        <translation>根前缀</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_base_mesh.cpp" line="29"/>
        <source>systemCoordinatesConverter</source>
        <translation>系统坐标转换器</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_base_mesh.cpp" line="30"/>
        <source>systemLengthUnit</source>
        <translation>系统长度单位</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_base_mesh.cpp" line="32"/>
        <source>Prefix for generating root labels name</source>
        <translation>生成根标签名称的前缀</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_base_mesh.cpp" line="33"/>
        <source>System length units to convert into while reading files</source>
        <translation>读取文件时转换的系统长度单位</translation>
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
        <translation>Y前向Z向上</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="47"/>
        <source>negZfwd_posYup</source>
        <translation>Z负向Y向上</translation>
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
        <location filename="../src/io_occ/io_occ_gltf_reader.cpp" line="19"/>
        <source>Ignore nodes without geometry(`Yes` by default)</source>
        <translation>忽略无几何体的节点（默认“是”）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_reader.cpp" line="22"/>
        <source>Use mesh name in case if node name is empty(`Yes` by default)</source>
        <translation>节点名称为空时使用网格名称（默认“是”）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_reader.cpp" line="33"/>
        <source>skipEmptyNodes</source>
        <translation>跳过空节点</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_reader.cpp" line="34"/>
        <source>useMeshNameAsFallback</source>
        <translation>使用网格名称作为后备</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccGltfWriter::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="31"/>
        <source>Source coordinate system transformation</source>
        <translation>源坐标系转换</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="33"/>
        <source>Target coordinate system transformation</source>
        <translation>目标坐标系转换</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="35"/>
        <source>Preferred transformation format for writing into glTF file</source>
        <translation>写入glTF文件的首选变换格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="37"/>
        <source>Export UV coordinates even if there is no mapped texture</source>
        <translation>即使无纹理贴图也导出UV</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="41"/>
        <source>Automatically choose most compact representation between Mat4 and TRS</source>
        <translation>自动选择Mat4与TRS间最紧凑的表示形式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="43"/>
        <source>4x4 transformation matrix</source>
        <translation>4x4变换矩阵</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="44"/>
        <source>Transformation decomposed into Translation vector, Rotation quaternion and Scale factor(T * R * S)</source>
        <translation>分解为平移向量、旋转四元数和缩放因子的变换（T * R * S）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="50"/>
        <source>Name format for exporting nodes</source>
        <translation>节点导出的名称格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="51"/>
        <source>Name format for exporting meshes</source>
        <translation>网格导出的名称格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="53"/>
        <source>Write image textures into target file.

If set to `false` then texture images will be written as separate files.

Applicable only if option `{0}` is set to `{1}`</source>
        <translation>将图像纹理写入目标文件。

    若设为“否”，纹理将保存为独立文件。

    仅在选项“{0}”设为“{1}”时生效</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="61"/>
        <source>Merge faces within a single part.

May reduce JSON size thanks to smaller number of primitive arrays</source>
        <translation>在单一部件内合并面。

    减少图元数组数量以缩小JSON文件大小</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="65"/>
        <source>Prefer keeping 16-bit indexes while merging face.

May reduce binary data size thanks to smaller triangle indexes.

Applicable only if option `{}` is on</source>
        <translation>合并面时优先保留16位索引。

    缩小三角形索引尺寸以减少二进制数据大小。

    仅在选项“{}”启用时生效</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="106"/>
        <source>inputCoordinateSystem</source>
        <translation>输入坐标系</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="107"/>
        <source>outputCoordinateSystem</source>
        <translation>输出坐标系</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="108"/>
        <source>transformationFormat</source>
        <translation>变换格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="109"/>
        <source>format</source>
        <translation>格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="110"/>
        <source>forceExportUV</source>
        <translation>强制导出UV</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="111"/>
        <source>nodeNameFormat</source>
        <translation>节点名称格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="112"/>
        <source>meshNameFormat</source>
        <translation>网格名称格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="113"/>
        <source>embedTextures</source>
        <translation>嵌入纹理贴图</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="114"/>
        <source>mergeFaces</source>
        <translation>合并面</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="115"/>
        <source>keepIndices16b</source>
        <translation>保持16位索引</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_gltf_writer.cpp" line="171"/>
        <source>Option supported from OpenCascade ≥ v7.6 [option={}, actual version={}]</source>
        <translation>选项支持OpenCascade ≥ v7.6 [选项={}, 实际版本={}]</translation>
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
        <location filename="../src/io_occ/io_occ_iges.cpp" line="27"/>
        <source>Manages the continuity of BSpline curves (IGES entities 106, 112 and 126) after translation to Open CASCADE (it requires that the curves in a model be at least C1 continuous; no such requirement is made by IGES).This parameter does not change the continuity of curves that are used in the construction of IGES BRep entities. In this case, the parameter does not influence the continuity of the resulting Open CASCADE curves (it is ignored).</source>
        <translation>管理BSpline曲线（IGES实体106、112和126）转换为OpenCASCADE后的连续性（要求模型中的曲线至少C1连续；而IGES无此要求）。此参数不会改变用于构建IGES BRep实体的曲线连续性，此时参数不影响生成的OpenCASCADE曲线连续性（将被忽略）。</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="38"/>
        <source>Preference for the computation of curves in case of 2D/3D inconsistency in an entity which has both 2D and 3D representations.

Concerned entity types are 141 (Boundary), 142 (CurveOnSurface) and 508 (Loop). These are entities representing a contour lying on a surface, which is translated to a TopoDS_Wire, formed by TopoDS_Edges. Each TopoDS_Edge must have a 3D curve and a 2D curve that reference the surface.

The processor also decides to re-compute either the 3D or the 2D curve even if both curves are translated successfully and seem to be correct, in case there is inconsistency between them. The processor considers that there is inconsistency if any of the following conditions is satisfied:
- the number of sub-curves in the 2D curve is different from the number of sub-curves in the 3D curve. This can be either due to different numbers of sub-curves given in the IGES file or because of splitting of curves during translation
- 3D or 2D curve is a Circular Arc (entity type 100) starting and ending in the same point (note that this case is incorrect according to the IGES standard)</source>
        <translation>在具有2D和3D表示的实体中，处理2D/3D不一致情况下曲线计算的优先级。

相关的实体类型有141（边界）、142（曲面上的曲线）和508（环）。这些实体表示位于表面上的轮廓，转换为由TopoDS_Edges形成的TopoDS_Wire。每个TopoDS_Edge必须有一个参考表面的3D曲线和2D曲线。

即使两条曲线都成功转换并且看起来正确，处理器也会在它们之间存在不一致的情况下重新计算3D或2D曲线。如果满足以下任何条件，处理器认为存在不一致：
- 2D曲线中的子曲线数量与3D曲线中的子曲线数量不同。这可能是由于IGES文件中给出的子曲线数量不同或在转换过程中曲线分裂造成的
- 3D或2D曲线是一个圆弧（实体类型100），起点和终点在同一点（注意，根据IGES标准这种情况是不正确的）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="57"/>
        <source>Read failed entities</source>
        <translation>读取实体失败</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="60"/>
        <source>Curves are taken as they are in the IGES file. C0 entities of Open CASCADE may be produced</source>
        <translation>直接采用IGES文件中的曲线，可能生成C0连续的OpenCASCADE实体</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="63"/>
        <source>If an IGES BSpline, Spline or CopiousData curve is C0 continuous, it is broken down into pieces of C1 continuous Geom_BSplineCurve</source>
        <translation>若IGES的BSpline、Spline或CopiousData曲线为C0连续，则分解为C1连续的Geom_BSplineCurve分段</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="66"/>
        <source>IGES Spline curves are broken down into pieces of C2 continuity. If C2 cannot be ensured, the Spline curves will be broken down into pieces of C1 continuity</source>
        <translation>IGES样条曲线分解为C2连续分段。若无法确保C2，则分解为C1连续分段</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="73"/>
        <source>Use the preference flag value in the entity&apos;s `Parameter Data` section</source>
        <translation>使用实体&quot;Parameter Data&quot;部分中的首选项标志值</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="74"/>
        <source>The 2D is used to rebuild the 3D in case of their inconsistency</source>
        <translation>当2D与3D不一致时，使用2D重建3D</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="75"/>
        <source>The 2D is always used to rebuild the 3D (even if 3D is present in the file)</source>
        <translation>始终使用2D重建3D（即使文件中存在3D数据）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="76"/>
        <source>The 3D is used to rebuild the 2D in case of their inconsistency</source>
        <translation>当2D与3D不一致时，使用3D重建2D</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="77"/>
        <source>The 3D is always used to rebuild the 2D (even if 2D is present in the file)</source>
        <translation>始终使用3D重建2D（即使文件中存在2D数据）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="89"/>
        <source>bsplineContinuity</source>
        <translation>BSpline连续性</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="90"/>
        <source>surfaceCurveMode</source>
        <translation>曲面曲线模式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="91"/>
        <source>readFaultyEntities</source>
        <translation>读取缺陷实体</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="92"/>
        <source>readOnlyVisibleEntities</source>
        <translation>仅读取可见实体</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccIgesWriter::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="157"/>
        <source>Indicates if planes should be saved as Bsplines or Planes (type 108). Writing p-curves on planes is disabled</source>
        <translation>指示平面应保存为B样条曲线或平面（类型108）。平面上的p曲线写入功能已禁用</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="160"/>
        <source>OpenCascade TopoDS_Faces will be translated into IGES 144 (Trimmed Surface) entities, no BRep entities will be written to the IGES file</source>
        <translation>OpenCascade的TopoDS_Faces将转换为IGES 144（裁剪曲面）实体，IGES文件不会写入BRep实体</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="163"/>
        <source>OpenCascade TopoDS_Faces will be translated into IGES 510 (Face) entities, the IGES file will contain BRep entities</source>
        <translation>OpenCascade的TopoDS_Faces将转换为IGES 510（面）实体，IGES文件将包含BRep实体</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_iges.cpp" line="176"/>
        <source>brepMode</source>
        <translation>BRep模式</translation>
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
</context>
<context>
    <name>Mayo::IO::OccObjReader::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_obj_reader.cpp" line="19"/>
        <source>Single precision flag for reading vertex data(coordinates)</source>
        <translation>顶点坐标读取的单精度标识</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_obj_reader.cpp" line="29"/>
        <source>singlePrecisionVertexCoords</source>
        <translation>单精度顶点坐标</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccObjWriterI18N</name>
    <message>
        <location filename="../src/io_occ/io_occ_obj_writer.cpp" line="31"/>
        <source>Source coordinate system transformation</source>
        <translation>源坐标系转换</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_obj_writer.cpp" line="34"/>
        <source>Target coordinate system transformation</source>
        <translation>目标坐标系转换</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_obj_writer.cpp" line="44"/>
        <source>inputCoordinateSystem</source>
        <translation>输入坐标系</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_obj_writer.cpp" line="45"/>
        <source>outputCoordinateSystem</source>
        <translation>输出坐标系</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccStepReader::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="36"/>
        <source>When reading AP 209 STEP files, allows selecting either only `design` or `analysis`, or both types of products for translation
Note that in AP 203 and AP214 files all products should be marked as `design`, so if this mode is set to `analysis`, nothing will be read</source>
        <translation>读取AP 209 STEP文件时，允许选择仅转换&apos;design&apos;、&apos;analysis&apos;或两者
    注意：AP 203和AP214文件中所有产品标记为&apos;design&apos;，若此模式设为&apos;analysis&apos;则不会读取内容</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="43"/>
        <source>Specifies which data should be read for the products found in the STEP file</source>
        <translation>指定STEP文件中产品的数据读取范围</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="47"/>
        <source>Specifies preferred type of representation of the shape of the product, in case if a STEP file contains more than one representation (i.e. multiple `PRODUCT_DEFINITION_SHAPE` entities) for a single product</source>
        <translation>当STEP文件包含多个形状表示时（如多个PRODUCT_DEFINITION_SHAPE实体），指定首选产品形状表示类型</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="53"/>
        <source>Defines whether shapes associated with the `PRODUCT_DEFINITION_SHAPE` entity of the product via `SHAPE_ASPECT` should be translated.
This kind of association was used for the representation of hybrid models (i.e. models whose shape is composed of different types of representations) in AP 203 files before 1998, but it is also used to associate auxiliary information with the sub-shapes of the part. Though STEP translator tries to recognize such cases correctly, this parameter may be useful to avoid unconditionally translation of shapes associated via `SHAPE_ASPECT` entities.</source>
        <translation>定义是否转换通过SHAPE_ASPECT关联到产品PRODUCT_DEFINITION_SHAPE实体的形状。
    此类关联曾用于1998年前AP 203文件的混合模型表示（由不同类型表示组成的模型），也可用于关联零件子形状的辅助信息。尽管STEP转换器会尝试正确识别，此参数可避免无条件转换SHAPE_ASPECT关联的形状。</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="64"/>
        <source>Indicates whether to read sub-shape names from &apos;Name&apos; attributes of STEP Representation Items</source>
        <translation>指示是否从STEP表示项的&apos;Name&apos;属性读取子形状名称</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="69"/>
        <source>Translate only products that have `PRODUCT_DEFINITION_CONTEXT` with field `life_cycle_stage` set to `design`</source>
        <translation>仅转换PRODUCT_DEFINITION_CONTEXT中生命周期阶段设为&apos;design&apos;的产品</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="72"/>
        <source>Translate only products that have `PRODUCT_DEFINITION_CONTEXT` with field `life_cycle_stage` set to `analysis`</source>
        <translation>仅转换PRODUCT_DEFINITION_CONTEXT中生命周期阶段设为&apos;analysis&apos;的产品</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="75"/>
        <source>Translates all products</source>
        <translation>转换所有产品</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="79"/>
        <source>Translate the assembly structure and shapes associated with parts only(not with sub-assemblies)</source>
        <translation>仅转换装配结构及零件关联形状（不包含子装配体）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="82"/>
        <source>Translate only the assembly structure without shapes(a structure of empty compounds). This mode can be useful as an intermediate step in applications requiring specialized processing of assembly parts</source>
        <translation>仅转换无形状的装配结构（空复合体结构）。此模式适用于需要专门处理装配部件的中间步骤</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="86"/>
        <source>Translate only shapes associated with the product, ignoring the assembly structure (if any). This can be useful to translate only a shape associated with specific product, as a complement to assembly mode</source>
        <translation>仅转换产品关联形状（忽略装配结构）。可作为装配模式的补充，用于转换特定产品的形状</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="90"/>
        <source>Translate both the assembly structure and all associated shapes. If both shape and sub-assemblies are associated with the same product, all of them are read and put in a single compound</source>
        <translation>转换装配结构及所有关联形状。若同一产品关联形状和子装配体，将全部读取至单个复合体</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="98"/>
        <source>Translate all representations(if more than one, put in compound)</source>
        <translation>转换所有表示形式（多个时将放入复合体）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="102"/>
        <source>Shift Japanese Industrial Standards</source>
        <translation>转换日本工业标准（JIS编码）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="103"/>
        <source>EUC(Extended Unix Code), multi-byte encoding primarily for Japanese, Korean, and simplified Chinese</source>
        <translation>EUC（扩展Unix编码），主要用于日文、韩文和简体中文的多字节编码</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="105"/>
        <source>GB(Guobiao) encoding for Simplified Chinese</source>
        <translation>GB（国标）简体中文编码</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="119"/>
        <source>productContext</source>
        <translation>产品上下文</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="120"/>
        <source>assemblyLevel</source>
        <translation>装配层级</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="121"/>
        <source>preferredShapeRepresentation</source>
        <translation>首选形状表示</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="122"/>
        <source>readShapeAspect</source>
        <translation>读取形状特征</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="123"/>
        <source>readSubShapesNames</source>
        <translation>读取子形状名称</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="124"/>
        <source>encoding</source>
        <translation>编码</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccStepWriter::Properties</name>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="236"/>
        <source>Version of schema used for the output STEP file</source>
        <translation>输出STEP文件使用的模式版本</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="239"/>
        <source>Defines a unit in which the STEP file should be written. If set to unit other than millimeter, the model is converted to these units during the translation</source>
        <translation>定义STEP文件写入单位（非毫米单位时模型将自动转换）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="245"/>
        <source>Parameter to write all free vertices in one SDR (name and style of vertex are lost) or each vertex in its own SDR (name and style of vertex are exported)</source>
        <translation>顶点聚合模式：单SDR输出（丢失名称样式）或多SDR输出（保留信息但增大文件）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="250"/>
        <source>All free vertices are united into one compound and exported in one shape definition representation (vertex name and style are lost)</source>
        <translation>合并所有自由顶点为单一复合体（丢失顶点名称和样式）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="253"/>
        <source>Each vertex is exported in its own `SHAPE DEFINITION REPRESENTATION`(vertex name and style are not lost, but the STEP file size increases)</source>
        <translation>每个顶点独立输出（保留名称样式但增大文件体积）</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="259"/>
        <source>Indicates whether parametric curves (curves in parametric space of surface) should be written into the STEP file.
It can be disabled in order to minimize the size of the resulting file.</source>
        <translation>指示是否应将参数曲线（曲面参数空间中的曲线）写入STEP文件。
可以禁用它以最小化生成文件的大小。</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="265"/>
        <source>Indicates whether to write sub-shape names to &apos;Name&apos; attributes of STEP Representation Items</source>
        <translation>是否将子形状名称写入STEP表示项</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="269"/>
        <source>Author attribute in STEP header</source>
        <translation>STEP文件中的作者属性</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="270"/>
        <source>Organization(of author) attribute in STEP header</source>
        <translation>STEP文件中的组织属性</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="271"/>
        <source>Originating system attribute in STEP header</source>
        <translation>STEP文件中的原始系统属性</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="272"/>
        <source>Description attribute in STEP header</source>
        <translation>STEP文件中的描述属性</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="290"/>
        <source>schema</source>
        <translation>协议模式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="291"/>
        <source>lengthUnit</source>
        <translation>长度单位</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="292"/>
        <source>assemblyMode</source>
        <translation>装配模式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="293"/>
        <source>freeVertexMode</source>
        <translation>自由顶点模式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="294"/>
        <source>writeParametericCurves</source>
        <translation>写入参数曲线</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="295"/>
        <source>writeSubShapesNames</source>
        <translation>写入子形状名称</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="296"/>
        <source>headerAuthor</source>
        <translation>头部作者</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="297"/>
        <source>headerOrganization</source>
        <translation>头部组织</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="298"/>
        <source>headerOriginatingSystem</source>
        <translation>头部原始系统</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_step.cpp" line="299"/>
        <source>headerDescription</source>
        <translation>头部描述</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OccStlWriterI18N</name>
    <message>
        <location filename="../src/io_occ/io_occ_stl.cpp" line="70"/>
        <source>targetFormat</source>
        <translation>目标格式</translation>
    </message>
    <message>
        <location filename="../src/io_occ/io_occ_stl.cpp" line="122"/>
        <location filename="../src/io_occ/io_occ_stl.cpp" line="125"/>
        <source>Not all BRep faces are meshed</source>
        <translation>未将全部BRep面网格化</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="59"/>
        <source>Ascii</source>
        <translation>ASCII</translation>
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
        <location filename="../src/io_occ/io_occ_vrml_writer.cpp" line="42"/>
        <source>shapeRepresentation</source>
        <translation>形状表示</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OffReaderI18N</name>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="183"/>
        <source>Can&apos;t open input file</source>
        <translation>无法打开输入文件</translation>
    </message>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="191"/>
        <location filename="../src/io_off/io_off_reader.cpp" line="211"/>
        <source>Unexpected end of file</source>
        <translation>意外的文件结尾</translation>
    </message>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="195"/>
        <source>Wrong header keyword(should be [C][N][4]OFF</source>
        <translation>错误的文件头关键字（应为[C][N][4]OFF）</translation>
    </message>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="216"/>
        <source>No vertex or face count</source>
        <translation>缺少顶点或面数量</translation>
    </message>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="237"/>
        <source>No vertex coordinates at current line</source>
        <translation>当前行无顶点坐标</translation>
    </message>
    <message>
        <location filename="../src/io_off/io_off_reader.cpp" line="262"/>
        <source>Inconsistent vertex count of face</source>
        <translation>面的顶点数量不一致</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::OffWriterI18N</name>
    <message>
        <location filename="../src/io_off/io_off_writer.cpp" line="45"/>
        <source>Failed to open file</source>
        <translation>打开文件失败</translation>
    </message>
</context>
<context>
    <name>Mayo::IO::PlyWriterI18N</name>
    <message>
        <location filename="../src/io_ply/io_ply_writer.cpp" line="67"/>
        <source>Line that will appear in header</source>
        <translation>文件头注释行</translation>
    </message>
    <message>
        <location filename="../src/io_ply/io_ply_writer.cpp" line="78"/>
        <source>targetFormat</source>
        <translation>目标格式</translation>
    </message>
    <message>
        <location filename="../src/io_ply/io_ply_writer.cpp" line="79"/>
        <source>writeColors</source>
        <translation>写入颜色</translation>
    </message>
    <message>
        <location filename="../src/io_ply/io_ply_writer.cpp" line="80"/>
        <source>defaultColor</source>
        <translation>默认颜色</translation>
    </message>
    <message>
        <location filename="../src/io_ply/io_ply_writer.cpp" line="81"/>
        <source>comment</source>
        <translation>注释</translation>
    </message>
    <message>
        <location filename="../src/io_ply/io_ply_writer.cpp" line="139"/>
        <source>Failed to open file</source>
        <translation>打开文件失败</translation>
    </message>
    <message>
        <location filename="../src/io_ply/io_ply_writer.cpp" line="152"/>
        <source>Unknown host endianness</source>
        <translation>未知主机字节序</translation>
    </message>
    <message>
        <location filename="messages.cpp" line="65"/>
        <source>Ascii</source>
        <translation>ASCII</translation>
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
        <location filename="../src/base/io_system.cpp" line="220"/>
        <source>Error during import of &apos;{}&apos;
{}</source>
        <translation>导入&apos;{}&apos;时出错：{}</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="230"/>
        <source>Unknown format</source>
        <translation>未知格式</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="236"/>
        <source>Reading file</source>
        <translation>正在读取文件</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="239"/>
        <source>No supporting reader</source>
        <translation>无支持的读取器</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="249"/>
        <source>File read problem</source>
        <translation>文件读取问题</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="258"/>
        <source>Transferring file</source>
        <translation>正在传输文件</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="262"/>
        <location filename="../src/base/io_system.cpp" line="388"/>
        <source>File transfer problem</source>
        <translation>文件传输问题</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="380"/>
        <source>No supporting writer</source>
        <translation>无支持的写入器</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="385"/>
        <source>Transfer</source>
        <translation>传输</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="392"/>
        <source>Write</source>
        <translation>写入</translation>
    </message>
    <message>
        <location filename="../src/base/io_system.cpp" line="395"/>
        <source>File write problem</source>
        <translation>文件写入问题</translation>
    </message>
</context>
<context>
    <name>Mayo::Main</name>
    <message>
        <location filename="../src/app/main.cpp" line="97"/>
        <source>Mayo the opensource 3D CAD viewer and converter</source>
        <translation>Mayo开源3D CAD查看与转换器</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="104"/>
        <source>Theme for the UI(classic|dark)</source>
        <translation>界面主题（classic|dark）</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="105"/>
        <source>name</source>
        <translation>名称</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="111"/>
        <source>Settings file(INI format) to load at startup</source>
        <translation>启动时加载的设置文件（INI格式）</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="112"/>
        <location filename="../src/app/main.cpp" line="119"/>
        <location filename="../src/cli/main.cpp" line="200"/>
        <location filename="../src/cli/main.cpp" line="213"/>
        <location filename="../src/cli/main.cpp" line="221"/>
        <location filename="../src/cli/main.cpp" line="228"/>
        <source>filepath</source>
        <translation>文件路径</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="118"/>
        <location filename="../src/cli/main.cpp" line="227"/>
        <source>Writes log messages into output file</source>
        <translation>将日志信息写入输出文件</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="125"/>
        <location filename="../src/cli/main.cpp" line="234"/>
        <source>Don&apos;t filter out debug log messages in release build</source>
        <translation>在发布版本中不过滤调试日志</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="131"/>
        <location filename="../src/cli/main.cpp" line="246"/>
        <source>Show detailed system information and quit</source>
        <translation>显示详细系统信息并退出</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="136"/>
        <location filename="../src/cli/main.cpp" line="251"/>
        <source>files</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="137"/>
        <source>Files to open at startup, optionally</source>
        <translation>启动时可选打开的文件</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="138"/>
        <location filename="../src/cli/main.cpp" line="253"/>
        <source>[files...]</source>
        <translation>[文件...]</translation>
    </message>
    <message>
        <source>Execute unit tests and exit application</source>
        <translation type="vanished">执行单元测试并退出</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="179"/>
        <location filename="../src/cli/main.cpp" line="299"/>
        <source>OpenCascade settings file doesn&apos;t exist or is not readable [path=%1]</source>
        <translation>OpenCascade设置文件不存在或不可读 [路径=%1]</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="186"/>
        <location filename="../src/cli/main.cpp" line="306"/>
        <source>OpenCascade settings file could not be loaded with QSettings [path=%1]</source>
        <translation>无法通过QSettings加载OpenCascade设置文件 [路径=%1]</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="330"/>
        <location filename="../src/cli/main.cpp" line="376"/>
        <source>Failed to load application settings file [path=%1]</source>
        <translation>无法加载应用程序设置文件 [路径=%1]</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="362"/>
        <location filename="../src/cli/main.cpp" line="400"/>
        <source>Failed to load translation file [path=%1]</source>
        <translation>无法加载翻译文件 [路径=%1]</translation>
    </message>
    <message>
        <location filename="../src/app/main.cpp" line="419"/>
        <source>Failed to load theme &apos;%1&apos;</source>
        <translation>无法加载主题&apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="181"/>
        <source>mayo-conv the opensource CAD converter</source>
        <translation>mayo-conv 开源CAD转换器</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="186"/>
        <source>Display help on commandline options</source>
        <translation>显示命令行帮助</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="192"/>
        <source>Display version information</source>
        <translation>显示版本信息</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="198"/>
        <source>Use settings file(INI format) for the conversion. When this option isn&apos;t specified then cached settings are used</source>
        <translation>使用INI格式设置文件进行转换（未指定时使用缓存设置）</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="206"/>
        <source>Cache settings file provided with --use-settings for further use</source>
        <translation>通过--use-settings提供的缓存设置文件供后续使用</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="212"/>
        <source>Write settings cache to an output file(INI format)</source>
        <translation>将设置缓存写入输出文件(INI格式)</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="219"/>
        <source>Export opened files into an output file, can be repeated for different formats(eg. -e file.stp -e file.igs...)</source>
        <translation>导出已打开文件到输出文件，可重复指定不同格式(如：-e file.stp -e file.igs...)</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="240"/>
        <source>Disable progress reporting in console output</source>
        <translation>禁用控制台进度报告</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="252"/>
        <source>Files to open(import)</source>
        <translation>要打开(导入)的文件</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="451"/>
        <source>Error when writing to &apos;%1&apos;</source>
        <translation>写入&apos;%1&apos;时发生错误</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="453"/>
        <source>Settings cache written to %1</source>
        <translation>设置缓存已写入%1</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="466"/>
        <source>No input files -&gt; nothing to export</source>
        <translation>无输入文件 -&gt; 无需导出</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="484"/>
        <source>Settings &apos;%1&apos; cached</source>
        <translation>设置&apos;%1&apos;已缓存</translation>
    </message>
    <message>
        <location filename="../src/cli/main.cpp" line="487"/>
        <source>No supplied settings to cache</source>
        <translation>未提供需要缓存的设置</translation>
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
        <translation>视窗(&amp;W)</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.ui" line="73"/>
        <source>&amp;Display</source>
        <translation>显示(&amp;D)</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.cpp" line="242"/>
        <location filename="../src/app/mainwindow.cpp" line="308"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.cpp" line="300"/>
        <source>Information</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.cpp" line="304"/>
        <source>Warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../src/app/mainwindow.cpp" line="312"/>
        <source>Question</source>
        <translation>问题</translation>
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
        <source>Angle: {0}{1}</source>
        <translation type="vanished">角度：{0}{1}</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="311"/>
        <location filename="../src/measure/measure_display.cpp" line="383"/>
        <location filename="../src/measure/measure_display.cpp" line="434"/>
        <location filename="../src/measure/measure_display.cpp" line="472"/>
        <source>{0}: {1}{2}</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="317"/>
        <source>{0}: {1}{2}&lt;br&gt;Point1: {3}&lt;br&gt;Point2: {4}</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="384"/>
        <source>Angle</source>
        <translation type="unfinished">角度</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="435"/>
        <source>Length</source>
        <translation>长度</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="473"/>
        <source>Area</source>
        <translation>面积</translation>
    </message>
    <message>
        <location filename="../src/measure/measure_display.cpp" line="533"/>
        <source>Min point: {0}&lt;br&gt;Max point: {1}&lt;br&gt;Size: {2} x {3} x {4}{5}&lt;br&gt;Volume: {6}{7}</source>
        <translation>最小点：{0}&lt;br&gt;最大点：{1}&lt;br&gt;尺寸：{2}×{3}×{4}{5}&lt;br&gt;体积：{6}{7}</translation>
    </message>
</context>
<context>
    <name>Mayo::Mesh_DocumentTreeNodeProperties</name>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="390"/>
        <source>NodeCount</source>
        <translation>节点数</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="391"/>
        <source>TriangleCount</source>
        <translation>三角网格数</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="392"/>
        <source>Area</source>
        <translation>面积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="393"/>
        <source>Volume</source>
        <translation>体积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="412"/>
        <source>Data</source>
        <translation type="unfinished">数据</translation>
    </message>
</context>
<context>
    <name>Mayo::PointCloud_DocumentTreeNodeProperties</name>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="443"/>
        <source>PointCount</source>
        <translation>点数</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="444"/>
        <source>HasColors</source>
        <translation>具有颜色</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="445"/>
        <source>CornerMin</source>
        <translation>最小角点</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="446"/>
        <source>CornerMax</source>
        <translation>最大角点</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="465"/>
        <source>Data</source>
        <translation type="unfinished">数据</translation>
    </message>
</context>
<context>
    <name>Mayo::PropertyEditorI18N</name>
    <message>
        <location filename="../src/app/property_editor_factory.cpp" line="194"/>
        <source>Choose color ...</source>
        <translation>选择颜色...</translation>
    </message>
</context>
<context>
    <name>Mayo::PropertyItemDelegate</name>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="79"/>
        <source>%1d </source>
        <translation>%1天 </translation>
    </message>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="82"/>
        <source>%1h </source>
        <translation>%1小时 </translation>
    </message>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="85"/>
        <source>%1min </source>
        <translation>%1分钟 </translation>
    </message>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="88"/>
        <source>%1s</source>
        <translation>%1秒</translation>
    </message>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="148"/>
        <location filename="../src/app/property_item_delegate.cpp" line="158"/>
        <source>%1%2</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/property_item_delegate.cpp" line="293"/>
        <source>ERROR no stringifier for property type &apos;%1&apos;</source>
        <translation>错误：属性类型‘%1’无字符串化方法</translation>
    </message>
</context>
<context>
    <name>Mayo::QStringUtils</name>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="47"/>
        <location filename="../src/app/qstring_utils.cpp" line="67"/>
        <source>(%1 %2 %3)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="81"/>
        <source>[%1; %2%3; %4]</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="102"/>
        <location filename="../src/app/qstring_utils.cpp" line="104"/>
        <location filename="../src/app/qstring_utils.cpp" line="106"/>
        <location filename="../src/app/qstring_utils.cpp" line="108"/>
        <location filename="../src/app/qstring_utils.cpp" line="110"/>
        <source>%1%2</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="102"/>
        <source>B</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="104"/>
        <source>KB</source>
        <translation>KB</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="106"/>
        <location filename="../src/app/qstring_utils.cpp" line="108"/>
        <location filename="../src/app/qstring_utils.cpp" line="110"/>
        <source>MB</source>
        <translation>MB</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="115"/>
        <location filename="../src/app/qstring_utils.cpp" line="123"/>
        <source>Yes</source>
        <translation>是</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="115"/>
        <location filename="../src/app/qstring_utils.cpp" line="121"/>
        <source>No</source>
        <translation>否</translation>
    </message>
    <message>
        <location filename="../src/app/qstring_utils.cpp" line="122"/>
        <source>Partially</source>
        <translation>部分</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetClipPlanes</name>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="14"/>
        <source>Edit clip planes</source>
        <translation>编辑剖面</translation>
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
        <translation></translation>
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
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="289"/>
        <source>Y </source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_clip_planes.ui" line="308"/>
        <source>Z </source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetExplodeAssembly</name>
    <message>
        <location filename="../src/app/widget_explode_assembly.ui" line="14"/>
        <source>Form</source>
        <translation>表单</translation>
    </message>
    <message>
        <location filename="../src/app/widget_explode_assembly.ui" line="42"/>
        <source>%</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetFileSystem</name>
    <message>
        <location filename="../src/app/widget_file_system.cpp" line="105"/>
        <source>%1
Size: %2
Last modified: %3</source>
        <translation>%1
        大小: %2
        最后修改: %3</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetGrid</name>
    <message>
        <location filename="../src/app/widget_grid.ui" line="14"/>
        <source>Form</source>
        <translation>表单</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="34"/>
        <source>Show Grid</source>
        <translation>显示格线</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="48"/>
        <source>Plane: XOY</source>
        <translation>平面: XOY</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="53"/>
        <source>Plane: ZOX</source>
        <translation>平面: ZOX</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="58"/>
        <source>Plane: YOZ</source>
        <translation>平面: YOZ</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="63"/>
        <source>Plane: Custom</source>
        <translation>平面: 自定义</translation>
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
        <translation>矩形</translation>
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
        <translation>旋转</translation>
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
        <translation>偏移</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="368"/>
        <location filename="../src/app/widget_grid.ui" line="397"/>
        <source>Origin</source>
        <translation>原点</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="433"/>
        <source>Radius</source>
        <translation>半径</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="463"/>
        <source>Radius Step</source>
        <translation>半径步长</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="483"/>
        <source>Division</source>
        <translation>分段</translation>
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
        <translation>第十色</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="650"/>
        <source>Mode</source>
        <translation>模式</translation>
    </message>
    <message>
        <location filename="../src/app/widget_grid.ui" line="661"/>
        <source>Lines</source>
        <translation>线</translation>
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
        <location filename="../src/app/widget_gui_document.cpp" line="97"/>
        <source>Fit All</source>
        <translation>适应全部</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="99"/>
        <source>Edit Grid</source>
        <translation>编辑工作平面格线</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="102"/>
        <source>Edit clip planes</source>
        <translation>编辑剖平面</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="105"/>
        <source>Explode assemblies</source>
        <translation>分解装配体</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="108"/>
        <source>Measure shapes</source>
        <translation>测量形状</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="325"/>
        <source>Isometric</source>
        <translation>等轴测</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="326"/>
        <source>Back</source>
        <translation>后视图</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="327"/>
        <source>Front</source>
        <translation>前视图</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="328"/>
        <source>Left</source>
        <translation>左视图</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="329"/>
        <source>Right</source>
        <translation>右视图</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="330"/>
        <source>Top</source>
        <translation>顶视图</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="331"/>
        <source>Bottom</source>
        <translation>底视图</translation>
    </message>
    <message>
        <location filename="../src/app/widget_gui_document.cpp" line="341"/>
        <source>&lt;b&gt;Left-click&lt;/b&gt;: popup menu of pre-defined views
&lt;b&gt;CTRL+Left-click&lt;/b&gt;: apply &apos;%1&apos; view</source>
        <translation>&lt;b&gt;左键单击&lt;/b&gt;：预定义视图菜单
        &lt;b&gt;Ctrl+左键单击&lt;/b&gt;：应用‘%1’视图</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetHomeFiles</name>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="48"/>
        <source>New Document</source>
        <translation>新建文档</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="49"/>
        <source>

Create and add an empty document where you can import files</source>
        <translation>

创建并添加空文档以导入文件</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="60"/>
        <source>Open Document(s)</source>
        <translation>打开文档</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="61"/>
        <source>

Select files to load and open as distinct documents</source>
        <translation>

选择文件作为独立文档加载并打开</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="129"/>
        <source>today %1</source>
        <translation>今天 %1</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="132"/>
        <source>yersterday %1</source>
        <translation>昨天 %1</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="136"/>
        <location filename="../src/app/widget_home_files.cpp" line="143"/>
        <source>%1 %2</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="139"/>
        <source>%1 days ago %2</source>
        <translation>%1 天前 %2</translation>
    </message>
    <message>
        <location filename="../src/app/widget_home_files.cpp" line="153"/>
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
读取时间：%5
</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetMainControl</name>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="14"/>
        <source>Form</source>
        <translation>表单</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="106"/>
        <source>Model tree</source>
        <translation>模型树</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="111"/>
        <source>Opened documents</source>
        <translation>已打开文档</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="116"/>
        <source>File system</source>
        <translation>文件系统</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.ui" line="134"/>
        <source>Close Left Side Bar</source>
        <translation>关闭左侧边栏</translation>
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
        <location filename="../src/app/widget_main_control.cpp" line="350"/>
        <source>Data</source>
        <translation>数据</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.cpp" line="331"/>
        <source>Graphics</source>
        <translation>图形</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.cpp" line="313"/>
        <source>%1(%2)</source>
        <translation type="unfinished">%1秒 {1(%2)?}</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.cpp" line="380"/>
        <source>Options</source>
        <translation>选项</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.cpp" line="429"/>
        <source>Document file `%1` has been changed since it was opened

Do you want to reload that document?

File: `%2`</source>
        <translation>文档文件`%1`自打开后已被修改

        是否重新加载该文档？

        文件：`%2`</translation>
    </message>
    <message>
        <location filename="../src/app/widget_main_control.cpp" line="436"/>
        <source>Question</source>
        <translation>问题</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetMainHome</name>
    <message>
        <location filename="../src/app/widget_main_home.ui" line="14"/>
        <source>Form</source>
        <translation>表单</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetMeasure</name>
    <message>
        <location filename="../src/app/widget_measure.ui" line="14"/>
        <source>Form</source>
        <translation>表单</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="32"/>
        <source>Area Unit</source>
        <translation>面积单位</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="40"/>
        <source>Square Millimeter(mm²)</source>
        <translation>平方毫米(mm²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="45"/>
        <source>Square Centimeter(cm²)</source>
        <translation>平方厘米(cm²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="50"/>
        <source>Square Meter(m²)</source>
        <translation>平方米(m²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="55"/>
        <source>Square Inch(in²)</source>
        <translation>平方英寸(in²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="60"/>
        <source>Square Foot(ft²)</source>
        <translation>平方英尺(ft²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="65"/>
        <source>Square Yard(yd²)</source>
        <translation>平方码(yd²)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="73"/>
        <source>Length Unit</source>
        <translation>长度单位</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="80"/>
        <source>Measure</source>
        <translation>测量</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="88"/>
        <source>Millimeter(mm)</source>
        <translation>毫米(mm)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="93"/>
        <source>Centimeter(cm)</source>
        <translation>厘米(cm)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="98"/>
        <source>Meter(m)</source>
        <translation>米(m)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="103"/>
        <source>Inch(in)</source>
        <translation>英寸(in)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="108"/>
        <source>Foot(ft)</source>
        <translation>英尺(ft)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="113"/>
        <source>Yard(yd)</source>
        <translation>码(英制长度单位)(yd)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="122"/>
        <source>Degree(°)</source>
        <translation>度(°)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="127"/>
        <source>Radian(rad)</source>
        <translation>弧度(rad)</translation>
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
        <location filename="../src/app/widget_measure.ui" line="196"/>
        <source>Angle</source>
        <translation>角度</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="201"/>
        <source>Length</source>
        <translation>长度</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="206"/>
        <source>Surface Area</source>
        <translation>表面积</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="211"/>
        <source>Bounding Box</source>
        <translation>边界框</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="220"/>
        <source>Cubic Millimeter(mm³)</source>
        <translation>立方毫米(mm³)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="225"/>
        <source>Cubic Centimeter(cm³)</source>
        <translation>立方厘米(cm³)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="230"/>
        <source>Cubic Meter(m³)</source>
        <translation>立方米(m³)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="235"/>
        <source>Cubic Inch(in³)</source>
        <translation>立方英寸(in³)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="240"/>
        <source>Cubic Foot(ft³)</source>
        <translation>立方英尺(ft³)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="245"/>
        <source>Liter(L)</source>
        <translation>升(L)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="250"/>
        <source>Imperial Gallon(GBgal)</source>
        <translation>英制加仑(GBgal)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.ui" line="255"/>
        <source>US Gallon(USgal)</source>
        <translation>美制加仑(USgal)</translation>
    </message>
    <message>
        <location filename="../src/app/widget_measure.cpp" line="429"/>
        <source>Select entities to measure</source>
        <translation>选择要测量的实体</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetModelTree</name>
    <message>
        <location filename="../src/app/widget_model_tree.ui" line="14"/>
        <source>Form</source>
        <translation>表单</translation>
    </message>
    <message>
        <location filename="../src/app/widget_model_tree.cpp" line="156"/>
        <source>Remove from document</source>
        <translation>从文档中移除</translation>
    </message>
    <message>
        <location filename="../src/app/widget_model_tree_builder.cpp" line="65"/>
        <source>&lt;unnamed&gt;</source>
        <translation>&lt;未命名&gt;</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetModelTreeBuilder_Xde</name>
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
    <message>
        <location filename="../src/app/widget_model_tree_builder_xde.cpp" line="67"/>
        <source>instanceNameFormat</source>
        <translation>实例名称格式</translation>
    </message>
    <message>
        <location filename="../src/app/widget_model_tree_builder_xde.cpp" line="125"/>
        <source>Show {}</source>
        <translation>显示 {}</translation>
    </message>
</context>
<context>
    <name>Mayo::WidgetPropertiesEditor</name>
    <message>
        <location filename="../src/app/widget_properties_editor.ui" line="14"/>
        <source>Form</source>
        <translation>表单</translation>
    </message>
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
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="59"/>
        <source>Name</source>
        <translation>名称</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="60"/>
        <source>Shape</source>
        <translation>形状</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="61"/>
        <source>XdeShape</source>
        <translation>Xde形状</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="62"/>
        <source>XdeLayer</source>
        <translation>Xde图层</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="63"/>
        <source>Color</source>
        <translation>颜色</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="64"/>
        <source>Location</source>
        <translation>位置</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="65"/>
        <source>Centroid</source>
        <translation>质心</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="66"/>
        <source>Area</source>
        <translation>面积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="67"/>
        <source>Volume</source>
        <translation>体积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="68"/>
        <source>MaterialDensity</source>
        <translation>材料密度</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="69"/>
        <source>MaterialName</source>
        <translation>材料名称</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="71"/>
        <source>ProductName</source>
        <translation>产品名称</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="72"/>
        <source>ProductColor</source>
        <translation>产品颜色</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="73"/>
        <source>ProductCentroid</source>
        <translation>产品质心</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="74"/>
        <source>ProductArea</source>
        <translation>产品面积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="75"/>
        <source>ProductVolume</source>
        <translation>产品体积</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="120"/>
        <source>Assembly</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="123"/>
        <source>Reference</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="126"/>
        <source>Component</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="129"/>
        <source>Compound</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="132"/>
        <source>Simple</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="135"/>
        <source>Sub</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="360"/>
        <source>Data</source>
        <translation type="unfinished">数据</translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="361"/>
        <source>Validation</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="362"/>
        <source>MetaData</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/document_tree_node_properties_providers.cpp" line="363"/>
        <source>ProductMetaData</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>OpenCascade::Aspect_HatchStyle</name>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="46"/>
        <source>Solid</source>
        <translation>实线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="47"/>
        <source>Horizontal</source>
        <translation>水平线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="48"/>
        <source>HorizontalSparse</source>
        <translation>稀疏水平线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="49"/>
        <source>Vertical</source>
        <translation>垂直线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="50"/>
        <source>VerticalSparse</source>
        <translation>稀疏垂直线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="51"/>
        <source>Diagonal45</source>
        <translation>45度对角线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="52"/>
        <source>Diagonal45Sparse</source>
        <translation>稀疏45度对角线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="53"/>
        <source>Diagonal135</source>
        <translation>135度对角线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="54"/>
        <source>Diagonal135Sparse</source>
        <translation>稀疏135度对角线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="55"/>
        <source>Grid</source>
        <translation>格线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="56"/>
        <source>GridSparse</source>
        <translation>稀疏网格</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="57"/>
        <source>GridDiagonal</source>
        <translation>对角格线</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="58"/>
        <source>GridDiagonalSparse</source>
        <translation>稀疏对角格线</translation>
    </message>
</context>
<context>
    <name>OpenCascade::Graphic3d_NameOfMaterial</name>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="19"/>
        <source>Brass</source>
        <translation>黄铜</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="20"/>
        <source>Bronze</source>
        <translation>青铜</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="21"/>
        <source>Copper</source>
        <translation>铜</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="22"/>
        <source>Gold</source>
        <translation>金</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="23"/>
        <source>Pewter</source>
        <translation>锡</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="24"/>
        <source>Plaster</source>
        <translation>石膏</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="25"/>
        <source>Plastic</source>
        <translation>塑料</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="26"/>
        <source>Silver</source>
        <translation>银</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="27"/>
        <source>Steel</source>
        <translation>钢</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="28"/>
        <source>Stone</source>
        <translation>石头</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="29"/>
        <source>ShinyPlastic</source>
        <translation>亮面塑料</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="30"/>
        <source>Satin</source>
        <translation>缎面</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="31"/>
        <source>Metalized</source>
        <translation>金属化</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="32"/>
        <source>NeonGnc</source>
        <translation>霓虹绿色</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="33"/>
        <source>Chrome</source>
        <translation>铬</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="34"/>
        <source>Aluminium</source>
        <translation>铝</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="35"/>
        <source>Obsidian</source>
        <translation>黑曜石</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="36"/>
        <source>NeonPhc</source>
        <translation>霓虹磷光</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="37"/>
        <source>Jade</source>
        <translation>玉</translation>
    </message>
    <message>
        <location filename="../src/base/occt_enums.cpp" line="38"/>
        <source>Default</source>
        <translation>默认</translation>
    </message>
</context>
</TS>

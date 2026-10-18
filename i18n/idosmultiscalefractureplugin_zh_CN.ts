<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN" sourcelanguage="en">
<context>
    <name>IDOSCurveFitEvaluationWidget</name>
    <message>
        <source>CSV files have a header row</source>
        <translation>CSV 文件包含表头</translation>
    </message>
    <message>
        <source>I confirm that both selected value columns represent the same quantity and use the common unit.</source>
        <translation>我确认两条曲线的数值列表示相同物理量，并使用下方的统一单位。</translation>
    </message>
    <message>
        <source>Export CSV...</source>
        <translation>导出 CSV...</translation>
    </message>
    <message>
        <source>Production Curve Fit Evaluation</source>
        <translation>生产曲线拟合评价</translation>
    </message>
    <message>
        <source>Read-only fit evaluation, not automatic history matching. Simulated values are linearly interpolated at observed times within the overlap; no extrapolation or index pairing. Times use UTC and millisecond resolution. Units are not converted.</source>
        <translation>只读拟合评价，不执行自动历史拟合。在重叠时间范围内，将模拟值线性插值到观测时间点；不外推，不按序号配对。时间采用 UTC，精度为毫秒。不进行单位换算。</translation>
    </message>
    <message>
        <source>Simulated curve</source>
        <translation>模拟曲线</translation>
    </message>
    <message>
        <source>Observed curve</source>
        <translation>观测曲线</translation>
    </message>
    <message>
        <source>Elapsed days</source>
        <translation>累计天数</translation>
    </message>
    <message>
        <source>ISO calendar dates / timestamps</source>
        <translation>ISO 日期 / 时间戳</translation>
    </message>
    <message>
        <source>CSV time format:</source>
        <translation>CSV 时间格式：</translation>
    </message>
    <message>
        <source>Day zero (UTC):</source>
        <translation>起始时刻（UTC）：</translation>
    </message>
    <message>
        <source>Common value unit:</source>
        <translation>统一数值单位：</translation>
    </message>
    <message>
        <source>Metric</source>
        <translation>指标</translation>
    </message>
    <message>
        <source>Value</source>
        <translation>数值</translation>
    </message>
    <message>
        <source>UTC time</source>
        <translation>UTC 时间</translation>
    </message>
    <message>
        <source>Simulated</source>
        <translation>模拟值</translation>
    </message>
    <message>
        <source>Observed</source>
        <translation>观测值</translation>
    </message>
    <message>
        <source>Error</source>
        <translation>误差</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation>评价</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <source>Browse...</source>
        <translation>浏览...</translation>
    </message>
    <message>
        <source>Source:</source>
        <translation>数据来源：</translation>
    </message>
    <message>
        <source>Summary series:</source>
        <translation>汇总曲线：</translation>
    </message>
    <message>
        <source>CSV file:</source>
        <translation>CSV 文件：</translation>
    </message>
    <message>
        <source>Time column (1-based):</source>
        <translation>时间列（从 1 开始）：</translation>
    </message>
    <message>
        <source>Value column (1-based):</source>
        <translation>数值列（从 1 开始）：</translation>
    </message>
    <message>
        <source>CSV file</source>
        <translation>CSV 文件</translation>
    </message>
    <message>
        <source>Select Curve CSV</source>
        <translation>选择曲线 CSV</translation>
    </message>
    <message>
        <source>CSV Files (*.csv *.CSV);;All Files (*)</source>
        <translation>CSV 文件 (*.csv *.CSV);;所有文件 (*)</translation>
    </message>
    <message>
        <source>Paired samples</source>
        <translation>配对样本数</translation>
    </message>
    <message>
        <source>RMSE</source>
        <translation>均方根误差（RMSE）</translation>
    </message>
    <message>
        <source>MAE</source>
        <translation>平均绝对误差（MAE）</translation>
    </message>
    <message>
        <source>MAPE (%)</source>
        <translation>平均绝对百分比误差（MAPE，%）</translation>
    </message>
    <message>
        <source>MAPE nonzero samples</source>
        <translation>MAPE 非零样本数</translation>
    </message>
    <message>
        <source>R squared</source>
        <translation>决定系数（R²）</translation>
    </message>
    <message>
        <source>Observed points outside overlap</source>
        <translation>重叠范围外观测点数</translation>
    </message>
    <message>
        <source>Undefined</source>
        <translation>未定义</translation>
    </message>
    <message>
        <source>Showing %1 of %2 paired samples; CSV export includes all. MAPE excludes zero observations; R squared is undefined for constant observations.</source>
        <translation>显示 %2 个配对样本中的前 %1 个；CSV 导出包含全部样本。MAPE 排除零观测值；观测值为常数时，R² 未定义。</translation>
    </message>
    <message>
        <source>Export Curve Evaluation</source>
        <translation>导出曲线评价</translation>
    </message>
    <message>
        <source>CSV Files (*.csv)</source>
        <translation>CSV 文件 (*.csv)</translation>
    </message>
    <message>
        <source>Cannot write evaluation CSV: %1</source>
        <translation>无法写入评价 CSV：%1</translation>
    </message>
    <message>
        <source>Evaluation CSV exported to %1.</source>
        <translation>评价 CSV 已导出到 %1。</translation>
    </message>
</context>
<context>
    <name>IDOSFieldDifferenceWidget</name>
    <message>
        <source>Field difference</source>
        <translation>属性场差异</translation>
    </message>
    <message>
        <source>I confirm matching physical quantities and units for inputs with unknown or different metadata.</source>
        <translation>对于元数据未知或不同的输入，我确认其物理量和单位一致。</translation>
    </message>
    <message>
        <source>Three-Field Difference</source>
        <translation>三场差异</translation>
    </message>
    <message>
        <source>Result = after - before on active cells. This is a property comparison, not a coupled three-field solver. No unit conversion is performed. Dynamic report steps are read without changing the displayed report step.</source>
        <translation>结果 = 计算后 - 计算前，仅计算活动单元。这是属性场比较，不执行三场耦合求解。不进行单位换算。读取动态报告步不会改变当前显示的报告步。</translation>
    </message>
    <message>
        <source>Grid:</source>
        <translation>网格：</translation>
    </message>
    <message>
        <source>Before property:</source>
        <translation>计算前属性：</translation>
    </message>
    <message>
        <source>Before report step:</source>
        <translation>计算前报告步：</translation>
    </message>
    <message>
        <source>After property:</source>
        <translation>计算后属性：</translation>
    </message>
    <message>
        <source>After report step:</source>
        <translation>计算后报告步：</translation>
    </message>
    <message>
        <source>Output name:</source>
        <translation>输出名称：</translation>
    </message>
    <message>
        <source>Calculate</source>
        <translation>计算</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <source>Static field</source>
        <translation>静态场</translation>
    </message>
    <message>
        <source>Created %1 active-cell differences. Minimum: %2; maximum: %3; mean: %4. Select the new property in the grid tree to display it.</source>
        <translation>已生成 %1 个活动单元的差异。最小值：%2；最大值：%3；平均值：%4。在网格树中选择新属性即可显示。</translation>
    </message>
</context>
<context>
    <name>IDOSMultiscaleFracturePlugin</name>
    <message>
        <source>Multiscale Fractures</source>
        <translation>多尺度裂缝</translation>
    </message>
    <message>
        <source>Multiscale fracture plugin</source>
        <translation type="vanished">多尺度裂缝插件</translation>
    </message>
    <message>
        <source>Equivalent Properties</source>
        <translation>等效计算</translation>
    </message>
    <message>
        <source>Equivalent Porosity/Permeability</source>
        <translation>等效孔渗</translation>
    </message>
    <message>
        <source>Equivalent Mechanics</source>
        <translation>等效力学</translation>
    </message>
    <message>
        <source>Grid Processing</source>
        <translation>网格处理</translation>
    </message>
    <message>
        <source>Grid Coarsening</source>
        <translation>网格粗化</translation>
    </message>
    <message>
        <source>Local Grid Extraction</source>
        <translation>局部切块</translation>
    </message>
    <message>
        <source>Model Construction</source>
        <translation>模型构建</translation>
    </message>
    <message>
        <source>EDFM Modeling</source>
        <translation>EDFM 建模</translation>
    </message>
    <message>
        <source>Connection Check</source>
        <translation>连接检查</translation>
    </message>
    <message>
        <source>Planned action; not implemented yet.</source>
        <translation>规划中的操作，暂未实现。</translation>
    </message>
    <message>
        <source>The command registry is unavailable.</source>
        <translation>命令注册表不可用。</translation>
    </message>
    <message>
        <source>Analysis command registration failed.</source>
        <translation>分析命令注册失败。</translation>
    </message>
    <message>
        <source>Analysis</source>
        <translation>分析</translation>
    </message>
    <message>
        <source>Three-Field Difference</source>
        <translation>三场差异</translation>
    </message>
    <message>
        <source>Curve Fit Evaluation</source>
        <translation>曲线拟合评价</translation>
    </message>
    <message>
        <source>Analysis commands are still alive, including undo or redo history. Close their project or clear the history before unloading.</source>
        <translation>仍有分析命令存活（包括撤销或重做历史）。卸载前请关闭对应工程或清空历史。</translation>
    </message>
    <message>
        <source>Close the analysis file dialog before unloading the plugin.</source>
        <translation>卸载插件前请关闭分析文件对话框。</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <source>Curve values and times must be finite and within the supported calendar range.</source>
        <translation>曲线数值和时间必须为有限值，且时间须在支持的日期范围内。</translation>
    </message>
    <message>
        <source>A curve contains duplicate timestamps. Resolve them before evaluation.</source>
        <translation>曲线存在重复时间戳，请处理后再评价。</translation>
    </message>
    <message>
        <source>A curve exceeds the 200000-point evaluation limit.</source>
        <translation>曲线超过 200000 个采样点的评价上限。</translation>
    </message>
    <message>
        <source>Select distinct time and value columns using 1-based indices.</source>
        <translation>请选择不同的时间列和数值列，列号从 1 开始。</translation>
    </message>
    <message>
        <source>Select calendar time or elapsed days with a valid UTC day-zero timestamp.</source>
        <translation>请选择日期时间，或为累计天数指定有效的 UTC 起始时刻。</translation>
    </message>
    <message>
        <source>Cannot open curve CSV: %1</source>
        <translation>无法打开曲线 CSV：%1</translation>
    </message>
    <message>
        <source>CSV line %1 has invalid quoting or missing columns.</source>
        <translation>CSV 第 %1 行引号格式无效或缺少列。</translation>
    </message>
    <message>
        <source>CSV line %1 has an invalid time or value.</source>
        <translation>CSV 第 %1 行的时间或数值无效。</translation>
    </message>
    <message>
        <source>Select an existing summary result and series.</source>
        <translation>请选择已有的汇总结果及曲线。</translation>
    </message>
    <message>
        <source>The summary series unit does not match the declared common unit. No conversion is performed.</source>
        <translation>汇总曲线单位与指定的统一单位不一致。不进行单位换算。</translation>
    </message>
    <message>
        <source>Summary timestamps and values have different lengths.</source>
        <translation>汇总时间戳与数值的数量不一致。</translation>
    </message>
    <message>
        <source>Declare a common value unit and confirm matching quantities and units for the selected curves.</source>
        <translation>请填写统一数值单位，并确认所选曲线的物理量和单位一致。</translation>
    </message>
    <message>
        <source>Both curves must contain at least one sample.</source>
        <translation>两条曲线都必须至少包含一个采样点。</translation>
    </message>
    <message>
        <source>A curve metric overflowed. Rescale the input values.</source>
        <translation>曲线评价指标溢出，请调整输入数值尺度。</translation>
    </message>
    <message>
        <source>The curves have no overlapping timestamps. Index-based pairing and extrapolation are not used.</source>
        <translation>两条曲线没有重叠时间点。不按序号配对，也不外推。</translation>
    </message>
    <message>
        <source>Evaluate curve fit</source>
        <translation>评价曲线拟合</translation>
    </message>
    <message>
        <source>Curve evaluation is read-only and has no undo operation.</source>
        <translation>曲线评价为只读操作，不支持撤销。</translation>
    </message>
    <message>
        <source>Evaluate time-aligned simulated and observed curves without changing project data.</source>
        <translation>评价时间对齐后的模拟曲线与观测曲线，不修改工程数据。</translation>
    </message>
    <message>
        <source>Field difference</source>
        <translation>属性场差异</translation>
    </message>
    <message>
        <source>Select a valid grid and two properties of that grid.</source>
        <translation>请选择有效网格及其两个属性。</translation>
    </message>
    <message>
        <source>The output name or grid ACTNUM is invalid.</source>
        <translation>输出名称或网格 ACTNUM 无效。</translation>
    </message>
    <message>
        <source>Both input properties must belong to the selected grid and match its dimensions.</source>
        <translation>两个输入属性必须属于所选网格，且维度与网格一致。</translation>
    </message>
    <message>
        <source>Select an existing report step for each dynamic input.</source>
        <translation>请为每个动态输入选择已有报告步。</translation>
    </message>
    <message>
        <source>An input property has an invalid value count.</source>
        <translation>输入属性的数值数量无效。</translation>
    </message>
    <message>
        <source>Confirm matching physical quantities and units. No unit conversion is performed.</source>
        <translation>请确认物理量和单位一致。不进行单位换算。</translation>
    </message>
    <message>
        <source>The cached result cannot be restored to its grid.</source>
        <translation>无法将缓存结果恢复到所属网格。</translation>
    </message>
    <message>
        <source>An active cell has a missing or nonfinite input or difference.</source>
        <translation>活动单元的输入或差值缺失或不是有限值。</translation>
    </message>
    <message>
        <source>No valid active cells are available for comparison.</source>
        <translation>没有可用于比较的有效活动单元。</translation>
    </message>
    <message>
        <source>The difference values could not be stored.</source>
        <translation>无法保存差异数值。</translation>
    </message>
    <message>
        <source>The difference result is unavailable.</source>
        <translation>差异结果不可用。</translation>
    </message>
    <message>
        <source>Subtract the before field from the after field on active cells.</source>
        <translation>在活动单元上用计算后属性场减去计算前属性场。</translation>
    </message>
    <message>
        <source>Multiscale Fractures</source>
        <translation>多尺度裂缝</translation>
    </message>
    <message>
        <source>Multiscale fracture plugin</source>
        <translation>多尺度裂缝插件</translation>
    </message>
    <message>
        <source>The output property is unavailable.</source>
        <translation>输出属性不可用。</translation>
    </message>
</context>
</TS>

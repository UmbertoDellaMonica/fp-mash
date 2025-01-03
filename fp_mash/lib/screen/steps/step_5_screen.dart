import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:fp_mash/services/mash_services_shell.dart';
import 'package:oktoast/oktoast.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:file_picker/file_picker.dart';

class Step5Screen extends StatefulWidget {
  const Step5Screen({super.key});

  @override
  _Step5ScreenState createState() => _Step5ScreenState();
}

class _Step5ScreenState extends State<Step5Screen> {
  String? filePath1;
  String? filePath2;
  String licenseOutput = '';
  String distanceResults = '';
  bool isLoading = false;

  final MashShellService _mashShellService = MashShellService();

  Future<void> _showMashLicense() async {
    setState(() {
      isLoading = true;
    });

    final output = await _mashShellService.showLicense();

    setState(() {
      licenseOutput = output;
      isLoading = false;
    });

    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: const Text('Mash License Information'),
          content: SingleChildScrollView(
            child: Text(
              licenseOutput,
              style: const TextStyle(fontSize: 14),
            ),
          ),
          actions: <Widget>[
            TextButton(
              onPressed: () {
                Navigator.of(context).pop();
              },
              child: const Text('Close'),
            ),
          ],
        );
      },
    );
  }

  Future<void> _pickFile(bool isFirstFile) async {
    try {
      FilePickerResult? result = await FilePicker.platform.pickFiles();

      if (result != null) {
        String? filePath = result.files.single.path;

        if (kDebugMode) {
          print("File Path : $filePath");
        }

        setState(() {
          if (isFirstFile) {
            filePath1 = filePath;
          } else {
            filePath2 = filePath;
          }
        });

        showToast(
          "File uploaded successfully!",
          duration: const Duration(seconds: 2),
          position: ToastPosition.bottom,
          backgroundColor: Colors.green,
          textStyle: const TextStyle(color: Colors.white),
        );
      } else {
        showToast(
          "Failed to determine save directory",
          duration: const Duration(seconds: 2),
          position: ToastPosition.bottom,
          backgroundColor: Colors.red,
          textStyle: const TextStyle(color: Colors.white),
        );
      }
    } catch (e) {
      showToast(
        "Failed to pick file: $e",
        duration: const Duration(seconds: 2),
        position: ToastPosition.bottom,
        backgroundColor: Colors.red,
        textStyle: const TextStyle(color: Colors.white),
      );
    }
  }

  Future<void> _calculateDistance() async {
    if (filePath1 != null && filePath2 != null) {
      setState(() {
        isLoading = true;
      });

      final results =
          await _mashShellService.calculateDistance(filePath1!, filePath2!);

      setState(() {
        distanceResults = results;
        isLoading = false;
      });
    } else {
      showToast(
        "Please upload both sketch files before calculating.",
        duration: const Duration(seconds: 2),
        position: ToastPosition.bottom,
        backgroundColor: Colors.orange,
        textStyle: const TextStyle(color: Colors.white),
      );
    }
  }

  void _showResultsGraph(String results) {
    List<String> ids = [];
    List<double> distances = [];
    List<double> pValues = [];
    List<double> numerators = [];
    List<double> denominators = [];

    for (String line in results.split("\n")) {
      List<String> parts = line.split("\t");
      if (parts.length >= 4) {
        ids.add("${parts[0]} - ${parts[1]}");
        distances.add(double.tryParse(parts[2].replaceAll('%', '')) ?? 0);
        pValues.add(double.tryParse(parts[3].replaceAll('%', '')) ?? 0);
        var numDenom = parts[4].split('/');
        numerators.add(double.tryParse(numDenom[0]) ?? 0);
        denominators.add(double.tryParse(numDenom[1]) ?? 0);
      }
    }

    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: const Text('Distance Results'),
          content: SingleChildScrollView(
            child: Column(
              children: List.generate(ids.length, (index) {
                return Column(
                  children: [
                    Text(
                      ids[index],
                      style: const TextStyle(
                          fontSize: 16, fontWeight: FontWeight.bold),
                    ),
                    const SizedBox(height: 10),
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                      children: [
                        ElevatedButton(
                          onPressed: () {
                            _showGraphDialog(
                              'Distance Graph',
                              LineChart(
                                LineChartData(
                                  gridData: const FlGridData(show: true),
                                  titlesData: const FlTitlesData(show: true),
                                  borderData: FlBorderData(show: true),
                                  lineBarsData: [
                                    LineChartBarData(
                                      spots: [
                                        FlSpot(index.toDouble(), distances[index]),
                                      ],
                                      isCurved: true,
                                      color: Colors.blueAccent,
                                    ),
                                  ],
                                ),
                              ),
                            );
                          },
                          child: const Text('Distance'),
                        ),
                        ElevatedButton(
                          onPressed: () {
                            _showGraphDialog(
                              'P-Value Graph',
                              LineChart(
                                LineChartData(
                                  gridData: const FlGridData(show: true),
                                  titlesData: const FlTitlesData(show: true),
                                  borderData: FlBorderData(show: true),
                                  lineBarsData: [
                                    LineChartBarData(
                                      spots: [
                                        FlSpot(index.toDouble(), pValues[index]),
                                      ],
                                      isCurved: true,
                                      color: Colors.greenAccent,
                                    ),
                                  ],
                                ),
                              ),
                            );
                          },
                          child: const Text('P-Value'),
                        ),
                        ElevatedButton(
                          onPressed: () {
                            _showGraphDialog(
                              'Numerator/Denominator Graph',
                              BarChart(
                                BarChartData(
                                  gridData: const FlGridData(show: true),
                                  titlesData: const FlTitlesData(show: true),
                                  borderData: FlBorderData(show: true),
                                  barGroups: [
                                    BarChartGroupData(
                                      x: index,
                                      barRods: [
                                        BarChartRodData(
                                          toY: numerators[index],
                                          color: Colors.blueAccent,
                                          width: 15,
                                          borderRadius: BorderRadius.zero,
                                        ),
                                        BarChartRodData(
                                          toY: denominators[index],
                                          color: Colors.orangeAccent,
                                          width: 15,
                                          borderRadius: BorderRadius.zero,
                                        ),
                                      ],
                                    ),
                                  ],
                                ),
                              ),
                            );
                          },
                          child: const Text('Numerator/Denominator'),
                        ),
                      ],
                    ),
                    const Divider(),
                  ],
                );
              }),
            ),
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(context).pop(),
              child: const Text('Close'),
            ),
          ],
        );
      },
    );
  }

  void _showGraphDialog(String title, Widget graph) {
    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: Text(title),
          content: Flex(
            direction: Axis.horizontal,
            children: [graph],
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(context).pop(),
              child: const Text('Close'),
            ),
          ],
        );
      },
    );
  }



  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Step 5 - Mash Calculate Distance'),
        actions: [
          IconButton(
            icon: const Icon(Icons.help_outline),
            onPressed: _showMashLicense,
            tooltip: 'Show Mash License',
          ),
        ],
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: SingleChildScrollView(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.start,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: <Widget>[
              const Divider(),
              const Text(
                'Mash Calculate Distance Between Genetic Sequences!',
                style: TextStyle(fontSize: 18.0, fontWeight: FontWeight.bold),
              ),
              const Text(
                'Upload the Sketch-Fingerprints Files generated in Step 4 to calculate the distance between two genetic sequences.',
                textAlign: TextAlign.center,
                style: TextStyle(fontSize: 18.0),
              ),
              const SizedBox(height: 20),
              Row(
                children: <Widget>[
                  Expanded(
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.center,
                      children: <Widget>[
                        const Text('Sketch File 1',
                            style: TextStyle(fontSize: 18.0)),
                        Tooltip(
                          message: 'Upload Sketch File 1',
                          child: ElevatedButton(
                            onPressed: () {
                              _pickFile(true);
                            },
                            child: const Text('Upload File'),
                          ),
                        ),
                        if (filePath1 != null) ...[
                          const SizedBox(height: 10),
                          Column(
                            children: [
                              const Icon(Icons.insert_drive_file, size: 40.0),
                              const SizedBox(height: 5),
                              Text(
                                filePath1!,
                                textAlign: TextAlign.center,
                                style: const TextStyle(fontSize: 12.0),
                                overflow: TextOverflow.ellipsis,
                              ),
                            ],
                          ),
                        ],
                      ],
                    ),
                  ),
                  const SizedBox(width: 20),
                  Expanded(
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.center,
                      children: <Widget>[
                        const Text('Sketch File 2',
                            style: TextStyle(fontSize: 18.0)),
                        Tooltip(
                          message: 'Upload Sketch File 2',
                          child: ElevatedButton(
                            onPressed: () {
                              _pickFile(false);
                            },
                            child: const Text('Upload File'),
                          ),
                        ),
                        if (filePath2 != null) ...[
                          const SizedBox(height: 10),
                          Column(
                            children: [
                              const Icon(Icons.insert_drive_file, size: 40.0),
                              const SizedBox(height: 5),
                              Text(
                                filePath2!,
                                textAlign: TextAlign.center,
                                style: const TextStyle(fontSize: 12.0),
                                overflow: TextOverflow.ellipsis,
                              ),
                            ],
                          ),
                        ],
                      ],
                    ),
                  ),
                ],
              ),
              const Divider(),
              ElevatedButton(
                onPressed: filePath1 != null && filePath2 != null && !isLoading
                    ? _calculateDistance
                    : null,
                child: isLoading
                    ? const CircularProgressIndicator()
                    : const Text('Calculate Distance'),
              ),
              const Divider(),
              if (distanceResults.isNotEmpty) ...[
                const Text(
                  'Result IDs:',
                  style: TextStyle(fontSize: 18.0, fontWeight: FontWeight.bold),
                ),
                ...distanceResults.split("\n").map((line) {
                  return Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Expanded(
                        child: Text(
                          line,
                          style: const TextStyle(fontSize: 16.0),
                          overflow: TextOverflow.ellipsis,
                        ),
                      ),
                      IconButton(
                        icon: const Icon(Icons.bar_chart),
                        onPressed: () => _showResultsGraph(distanceResults),
                      ),
                    ],
                  );
                }).toList(),
              ],
            ],
          ),
        ),
      ),
    );
  }
}

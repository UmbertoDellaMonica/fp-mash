import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';
import 'package:oktoast/oktoast.dart';
import 'package:path_provider/path_provider.dart';
import 'dart:io';

class Step3Screen extends StatefulWidget {
  const Step3Screen({super.key});

  @override
  _Step3ScreenState createState() => _Step3ScreenState();
}

class _Step3ScreenState extends State<Step3Screen> {
  bool isStep3Completed = false;
  String? filePath1;
  String? filePath2;

  /// Parameters for lyn2vec operation
  String selectedOperation = 'basic';
  bool revComb = false;
  String selectedFactorization = 'CFL';
  int nValue = 1;

  /// Available options for lyn2vec operation and factorization
  final List<String> lyn2vecOperations = ['basic', 'generalized', 'mapping'];
  final List<String> factorizationOptions = [
    'CFL',
    'ICFL',
    'CFL_ICFL-10',
    'CFL_ICFL-20',
    'CFL_ICFL-30',
    'CFL_COMB',
    'ICFL_COMB',
    'CFL_ICFL_COMB-10',
    'CFL_ICFL_COMB-20',
    'CFL_ICFL_COMB-30'
  ];

  Future<void> _pickFile(bool isFirstFile) async {
    try {
      FilePickerResult? result = await FilePicker.platform.pickFiles();

      if (result != null) {
        String? filePath = result.files.single.path;
        Directory? saveDir;

        if (Platform.isMacOS) {
          saveDir = await getApplicationSupportDirectory();
        } else if (Platform.isLinux) {
          saveDir = await getApplicationDocumentsDirectory();
        } else if (Platform.isWindows) {
          saveDir = await getApplicationSupportDirectory();
        }

        if (saveDir != null) {
          String savePath = '${saveDir.path}/${filePath!.split('/').last}';

          setState(() {
            if (isFirstFile) {
              filePath1 = savePath;
            } else {
              filePath2 = savePath;
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

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Step 3 - Lyn2vec Fingerprints'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: SingleChildScrollView(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.start,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: <Widget>[
              const Text(
                'Upload the files splitted from Step 2. For each file generate a fingerprint file',
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
                        const Text('Genetic Sequence 1 Splitted by k-mers', style: TextStyle(fontSize: 18.0)),
                        ElevatedButton(
                          onPressed: () {
                            _pickFile(true);
                          },
                          child: const Text('Upload File'),
                        ),
                        if (filePath1 != null) ...[
                          const SizedBox(height: 10),
                          const Icon(Icons.description, size: 40, color: Colors.blue),
                          const SizedBox(height: 10),
                          Text(
                            filePath1!.split('/').last,
                            textAlign: TextAlign.center,
                            style: const TextStyle(fontSize: 16.0),
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
                        const Text('Genetic Sequence 2 Splitted by k-mers', style: TextStyle(fontSize: 18.0)),
                        ElevatedButton(
                          onPressed: () {
                            _pickFile(false);
                          },
                          child: const Text('Upload File'),
                        ),
                        if (filePath2 != null) ...[
                          const SizedBox(height: 10),
                          const Icon(Icons.description, size: 40, color: Colors.blue),
                          const SizedBox(height: 10),
                          Text(
                            filePath2!.split('/').last,
                            textAlign: TextAlign.center,
                            style: const TextStyle(fontSize: 16.0),
                          ),
                        ],
                      ],
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 30),
              const Text(
                'Lyn2vec Parameters',
                style: TextStyle(fontSize: 18.0, fontWeight: FontWeight.bold),
              ),
              const SizedBox(height: 10),
              // Dropdown for selecting lyn2vec operation
              DropdownButton<String>(
                value: selectedOperation,
                items: lyn2vecOperations.map((String operation) {
                  return DropdownMenuItem<String>(
                    value: operation,
                    child: Text(operation),
                  );
                }).toList(),
                onChanged: (String? newValue) {
                  setState(() {
                    selectedOperation = newValue!;
                  });
                },
                hint: const Text('Select Operation Type'),
              ),
              const SizedBox(height: 20),
              // Checkbox for selecting rev_comb
              Row(
                mainAxisAlignment: MainAxisAlignment.start,
                children: [
                  const Text('Reverse and Complement:'),
                  Checkbox(
                    value: revComb,
                    onChanged: (bool? value) {
                      setState(() {
                        revComb = value!;
                      });
                    },
                  ),
                ],
              ),
              const SizedBox(height: 20),
              // Dropdown for selecting type_factorization
              DropdownButton<String>(
                value: selectedFactorization,
                items: factorizationOptions.map((String factorization) {
                  return DropdownMenuItem<String>(
                    value: factorization,
                    child: Text(factorization),
                  );
                }).toList(),
                onChanged: (String? newValue) {
                  setState(() {
                    selectedFactorization = newValue!;
                  });
                },
                hint: const Text('Select Factorization Type'),
              ),
              const SizedBox(height: 20),
              // Text field for entering n value
              TextField(
                keyboardType: TextInputType.number,
                decoration: const InputDecoration(
                  border: OutlineInputBorder(),
                  labelText: 'Enter n value (Optional)',
                  hintText: '1',
                ),
                onChanged: (String value) {
                  setState(() {
                    nValue = int.tryParse(value) ?? 1;
                  });
                },
              ),
              const SizedBox(height: 30),
              Row(
                mainAxisAlignment: MainAxisAlignment.end,
                children: [
                  ElevatedButton(
                    onPressed: () {
                      // TODO: Implement the following actions:
                      // - Retrieve the file splitted_1.txt for genetic sequence 1
                      // - Retrieve the file splitted_2.txt for genetic sequence 2
                      // - Call lyn2vec to generate the fingerprints using Python code with the selected parameters
                      // - If everything is successful, display a success toast

                      setState(() {
                        isStep3Completed = true;
                      });
                    },
                    child: const Text('Generate FingerPrints Sequence'),
                  ),
                  const SizedBox(width: 20),
                  ElevatedButton(
                    onPressed: isStep3Completed ? () {
                      // Navigate to the next step
                    } : null,
                    child: const Text('Next >'),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}

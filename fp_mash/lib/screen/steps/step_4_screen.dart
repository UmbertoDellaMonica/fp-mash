import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';
import 'package:fp_mash/services/directory_service.dart';
import 'package:fp_mash/services/mash_services_shell.dart';
import 'package:oktoast/oktoast.dart';
import 'dart:io';

class Step4Screen extends StatefulWidget {
  const Step4Screen({super.key});

  @override
  _Step4ScreenState createState() => _Step4ScreenState();
}

class _Step4ScreenState extends State<Step4Screen> {
  bool isStep4Completed = false;

  String? filePath1;
  String? filePath2;

  String? sketchFilePath1;
  String? sketchFilePath2;

  final MashShellService _mashShellService = MashShellService();

  final DirectoryService _directoryService = DirectoryService();

  String licenseOutput = '';
  bool isLoading = false;

  Future<void> _showMashLicense() async {
    setState(() {
      isLoading = true;
    });

    final output = await _mashShellService.showLicense();

    setState(() {
      licenseOutput = output;
      isLoading = false;
    });

    // Show a custom, scrollable toast that lasts indefinitely
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

  Future<void> _generateSketchFile(bool isFirstFile) async {
    String outputDirectory = await _directoryService.getCommonDirectoryPath();
    String step4Directory = _directoryService.step4Directory;

    String stepDirectory = '$outputDirectory$step4Directory';
    await _directoryService.createStepDirectory('step4_mash_sketch');

    String inputFilePath = isFirstFile ? filePath1! : filePath2!;

    // Ottieni il nome del file senza il percorso
    String fileName = inputFilePath.split('/').last;

    // Trova l'ultimo punto per rimuovere l'estensione attuale
    int extensionIndex = fileName.lastIndexOf('.');
    String baseFileName = (extensionIndex == -1)
        ? fileName
        : fileName.substring(0, extensionIndex);

    // Aggiungi l'estensione .msh
    String outputFileName = '$baseFileName.msh';

    // Crea il percorso completo per il file di output
    String outputFilePath = '$stepDirectory/$outputFileName';

    // Esegui il comando per generare gli sketch fingerprint
    String result = await _mashShellService.generateSketchFingerPrints(
        inputFilePath, outputFilePath);

    setState(() {
      if (isFirstFile) {
        sketchFilePath1 = outputFilePath;
      } else {
        sketchFilePath2 = outputFilePath;
      }
    });

    showToast(
      "Sketch file generated successfully at $outputFilePath",
      duration: const Duration(seconds: 2),
      position: ToastPosition.bottom,
      backgroundColor: Colors.green,
      textStyle: const TextStyle(color: Colors.white),
    );
  }

  bool canProceedToNextStep = true;

  Future<bool> _checkFilesExist() async {
    if (sketchFilePath1 != null && sketchFilePath2 != null) {
      File file1 = File(sketchFilePath1!);
      File file2 = File(sketchFilePath2!);

      bool file1Exists = await file1.exists();
      bool file2Exists = await file2.exists();

      if (file1Exists && file2Exists) {
        return true;
      } else {
        showToast(
          "One or both files do not exist.",
          duration: const Duration(seconds: 2),
          position: ToastPosition.bottom,
          backgroundColor: Colors.red,
          textStyle: const TextStyle(color: Colors.white),
        );
        return false;
      }
    }
    return false;
  }

  Future<void> _goToNextStep() async {
    await _directoryService
        .createStepDirectory(_directoryService.step4Directory);
    Navigator.pushNamed(context, '/step5');
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Step 4 - Mash Generate Sketch File'),
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
                'Mash Upload File and Generate Sketch-FingerPrints File!',
                style: TextStyle(fontSize: 18.0, fontWeight: FontWeight.bold),
              ),
              const Text(
                'Upload the files splitted from Step 2. For each file generate a Sketch-Fingerprint file',
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
                        const Text('Genetic Sequence 1',
                            style: TextStyle(fontSize: 18.0)),
                        Tooltip(
                          message: 'Carica il file di sequenza genetica 1',
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
                              const Icon(Icons.insert_drive_file, size: 40.0, color: Colors.green,),
                              const SizedBox(height: 5),
                              Text(
                                filePath1!,
                                textAlign: TextAlign.center,
                                style: const TextStyle(fontSize: 12.0),
                                overflow: TextOverflow.ellipsis,
                              ),
                              const SizedBox(height: 10),
                              Tooltip(
                                message:
                                    'Genera Sketch FingerPrint File per Sequenza Genetica 1',
                                child: ElevatedButton(
                                  onPressed: () {
                                    _generateSketchFile(true);
                                  },
                                  child:
                                      const Text('Generate Fingerprints File'),
                                ),
                              ),
                            ],
                          ),
                        ],
                        if (sketchFilePath1 != null) ...[
                          const SizedBox(height: 10),
                          Column(
                            children: [
                              const Icon(Icons.description, size: 40.0, color: Colors.blueAccent,),
                              const SizedBox(height: 5),
                              Text(
                                'Generated: $sketchFilePath1',
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
                        const Text('Genetic Sequence 2',
                            style: TextStyle(fontSize: 18.0)),
                        Tooltip(
                          message: 'Carica il file di sequenza genetica 2',
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
                              const Icon(Icons.insert_drive_file, size: 40.0, color: Colors.green,),
                              const SizedBox(height: 5),
                              Text(
                                filePath2!,
                                textAlign: TextAlign.center,
                                style: const TextStyle(fontSize: 12.0),
                                overflow: TextOverflow.ellipsis,
                              ),
                              const SizedBox(height: 10),
                              Tooltip(
                                message:
                                    'Genera Sketch FingerPrint File per Sequenza Genetica 2',
                                child: ElevatedButton(
                                  onPressed: () {
                                    _generateSketchFile(false);
                                  },
                                  child:
                                      const Text('Generate Fingerprints File'),
                                ),
                              ),
                            ],
                          ),
                        ],
                        if (sketchFilePath2 != null) ...[
                          const SizedBox(height: 10),
                          Column(
                            children: [
                              const Icon(Icons.description, size: 40.0, color: Colors.blueAccent,),
                              const SizedBox(height: 5),
                              Text(
                                'Generated: $sketchFilePath2',
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
                onPressed: () async {
                  canProceedToNextStep = await _checkFilesExist();
                  if (canProceedToNextStep) {
                    await _goToNextStep();
                  }
                },
                child: const Text('Next >'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

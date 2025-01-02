import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';
import 'package:fp_mash/services/directory_service.dart';
import 'package:fp_mash/services/dsk_services_shell.dart';
import 'package:oktoast/oktoast.dart';
import 'package:path_provider/path_provider.dart';
import 'dart:io';

class Step1Screen extends StatefulWidget {
  @override
  _Step1ScreenState createState() => _Step1ScreenState();
}

class _Step1ScreenState extends State<Step1Screen> {
  bool isStep1Completed = false;

  String? filePath1;
  String? filePath2;

  String? h5FilePath1;
  String? h5FilePath2;

  String? filePath1ConversionFasta;
  String? filePath2ConversionFasta;

  String dskOutput = '';

  bool isLoading = false;

  final DskShellService _dskShellService = DskShellService();

  final DirectoryService _directoryService = DirectoryService();

  /// Upload File from the directory 
  Future<void> _pickFile(bool isFirstFile) async {
    try {
      FilePickerResult? result = await FilePicker.platform.pickFiles();

      if (result != null) {
        String? filePath = result.files.single.path;

        print("File Path : $filePath");

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

  /// Funzione per la generazione del File H5 
  Future<void> _generateH5File(bool isFirstFile) async {
    setState(() {
      isLoading = true;
    });

    String inputFilePath = isFirstFile ? filePath1! : filePath2!;
    String inputDirectory =
        Directory(inputFilePath).parent.path; // Ottieni la cartella del file
    String outputDirectory =
        '$inputDirectory/step1_dsk'; // Crea la cartella 'step1_dsk' dentro la cartella del file

    // Genera il file HDF5
    String result =
        await _dskShellService.generateH5File(inputFilePath, outputDirectory);

    setState(() {
      isLoading = false;
      if (result.startsWith('Error:') || result.startsWith('Exception:')) {
        showToast(
          result,
          position: ToastPosition.bottom,
          backgroundColor: Colors.red,
          textStyle: const TextStyle(color: Colors.white),
        );
      } else {
        showToast(
          "File HDF5 generato con successo!",
          duration: const Duration(seconds: 3),
          position: ToastPosition.bottom,
          backgroundColor: Colors.green,
          textStyle: const TextStyle(color: Colors.white),
        );
        if (isFirstFile) {
          print("Output Directory : $outputDirectory");
          h5FilePath1 =
              '$outputDirectory/${filePath1!.split('/').last.split('.').first}.h5';
        } else {
          h5FilePath2 =
              '$outputDirectory/${filePath2!.split('/').last.split('.').first}.h5';
        }
      }
    });
  }

  /// Funzione per la conversione HDF5 in FASTA
  Future<void> _convertH5ToFasta(String h5FilePath, bool isFirstFile) async {
    setState(() {
      isLoading = true;
    });

    String inputDirectory = Directory(h5FilePath).parent.path;
    String outputFastaFile = '$inputDirectory/${h5FilePath.split('/').last.split('.').first}-extract.fasta';

    // Esegui il comando dsk2ascii per la conversione
    String result = await _dskShellService.convertH5ToFasta(h5FilePath, outputFastaFile);

    setState(() {
      isLoading = false;
      if (result.startsWith('Error:') || result.startsWith('Exception:')) {
        showToast(
          result,
          position: ToastPosition.bottom,
          backgroundColor: Colors.red,
          textStyle: const TextStyle(color: Colors.white),
        );
      } else {
        showToast(
          "File FASTA estratto con successo!",
          duration: const Duration(seconds: 3),
          position: ToastPosition.bottom,
          backgroundColor: Colors.green,
          textStyle: const TextStyle(color: Colors.white),
        );
        if (isFirstFile) {
          filePath1ConversionFasta = outputFastaFile;
        } else {
          filePath2ConversionFasta = outputFastaFile;
        }
      }
    });
  }



  /// Verifica dello Step1 se è completo oppure no 
  Future<void> _verifyStep1() async {
    if (filePath1ConversionFasta != null && filePath2ConversionFasta != null) {
      showToast(
        "Premi Next > per passare allo step successivo",
        duration: const Duration(seconds: 3),
        position: ToastPosition.bottom,
        backgroundColor: Colors.green,
        textStyle: const TextStyle(color: Colors.white),
      );
      setState(() {
        isStep1Completed = true;
      });
    } else {
      showToast(
        "Non abbiamo eseguito tutti i passaggi",
        duration: const Duration(seconds: 3),
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
        title: const Text('Step 1'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: SingleChildScrollView(
          // Render content as scrollable
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              const Text(
                'Insert the FASTA genetic sequence to extract k-mers + frequency using DSK.',
                textAlign: TextAlign.center,
                style: TextStyle(fontSize: 18.0),
              ),
              const SizedBox(height: 20),
              Row(
                children: <Widget>[
                  // Sequence Genetic 1 - Insert Input
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
                          const Icon(Icons.description,
                              size: 40, color: Colors.blue),
                          const SizedBox(height: 10),
                          Text(
                            filePath1!,
                            textAlign: TextAlign.center,
                            style: const TextStyle(fontSize: 16.0),
                          ),
                          Tooltip(
                            message: 'Genera il file HDF5 dalla sequenza genetica 1',
                            child: ElevatedButton(
                              onPressed: () {
                                _generateH5File(true);
                              },
                              child: const Text('Generate HDF5 File'),
                            ),
                          ),
                          if (h5FilePath1 != null) ...[
                            const SizedBox(height: 10),
                            const Icon(Icons.file_copy,
                                size: 40, color: Colors.green),
                            const SizedBox(height: 10),
                            Text(
                              h5FilePath1!,
                              textAlign: TextAlign.center,
                              style: const TextStyle(fontSize: 16.0),
                            ),
                            // Aggiungi il bottone per la conversione da H5 a FASTA
                            Tooltip(
                              message: 'Converti il file HDF5 in file FASTA',
                              child: ElevatedButton(
                                onPressed: () {
                                  _convertH5ToFasta(h5FilePath1!, true);
                                },
                                child: const Text('Convert H5 to FASTA'),
                              ),
                            ),
                            if (filePath1ConversionFasta != null) ...[
                              const SizedBox(height: 10),
                              const Icon(Icons.file_copy,
                                  size: 40, color: Colors.orange),
                              const SizedBox(height: 10),
                              Text(
                                filePath1ConversionFasta!,
                                textAlign: TextAlign.center,
                                style: const TextStyle(fontSize: 16.0),
                              ),
                            ],
                          ],
                        ],
                      ],
                    ),
                  ),
                  const SizedBox(width: 20),

                  // Sequence Genetic 2 - Insert Input
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
                          const Icon(Icons.description,
                              size: 40, color: Colors.blue),
                          const SizedBox(height: 10),
                          Text(
                            filePath2!,
                            textAlign: TextAlign.center,
                            style: const TextStyle(fontSize: 16.0),
                          ),
                          Tooltip(
                            message: 'Genera il file HDF5 dalla sequenza genetica 2',
                            child: ElevatedButton(
                              onPressed: () {
                                _generateH5File(false);
                              },
                              child: const Text('Generate HDF5 File'),
                            ),
                          ),
                          if (h5FilePath2 != null) ...[
                            const SizedBox(height: 10),
                            const Icon(Icons.file_copy,
                                size: 40, color: Colors.green),
                            const SizedBox(height: 10),
                            Text(
                              h5FilePath2!,
                              textAlign: TextAlign.center,
                              style: const TextStyle(fontSize: 16.0),
                            ),
                            // Aggiungi il bottone per la conversione da H5 a FASTA
                            Tooltip(
                              message: 'Converti il file HDF5 in file FASTA',
                              child: ElevatedButton(
                                onPressed: () {
                                  _convertH5ToFasta(h5FilePath2!, false);
                                },
                                child: const Text('Convert H5 to FASTA'),
                              ),
                            ),
                            if (filePath2ConversionFasta != null) ...[
                              const SizedBox(height: 10),
                              const Icon(Icons.file_copy,
                                  size: 40, color: Colors.orange),
                              const SizedBox(height: 10),
                              Text(
                                filePath2ConversionFasta!,
                                textAlign: TextAlign.center,
                                style: const TextStyle(fontSize: 16.0),
                              ),
                            ],
                          ],
                        ],
                      ],
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 20),
              Tooltip(
                message: 'Visualizza l\'aiuto per il comando DSK',
                child: ElevatedButton(
                  onPressed: _verifyStep1,
                  child: const Text('Verify Step1'),
                ),
              ),
              const SizedBox(height: 20),
              if (isStep1Completed)
                Tooltip(
                  message: 'Passa allo step successivo',
                  child: ElevatedButton(
                    onPressed: () async {

                      await _directoryService.createStepDirectory(_directoryService.step2Directory);

                      Navigator.pushNamed(context, '/step2');
                    },
                    child: const Text('Next >'),
                  ),
                ),
              /// TODO : Delete this Step 
              ElevatedButton(
                onPressed: () async {

                  await _directoryService.createStepDirectory(_directoryService.step2Directory);

                  // Navigate to the next step
                  Navigator.pushNamed(context, '/step2');
                },
                child: const Text('Next >'),
              ),
              const SizedBox(height: 20),
              if (isLoading) const CircularProgressIndicator(),
            ],
          ),
        ),
      ),
    );
  }
}

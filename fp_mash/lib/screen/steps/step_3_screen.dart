import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:file_picker/file_picker.dart';
import 'package:fp_mash/services/directory_service.dart';
import 'package:fp_mash/services/lyn2vec_services_shell.dart';
import 'package:oktoast/oktoast.dart';
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

  String? h5FilePath1;
  String? h5FilePath2;

  /// Aggiungi il servizio Lyn2vecShellService
  final Lyn2vecShellService _lyn2vecShellService = Lyn2vecShellService();

  /// Directory Service
  final DirectoryService _directoryService = DirectoryService();

  String licenseOutput = '';
  bool isLoading = false;

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

  Future<void> _showLyn2vecHelp() async {
    setState(() {
      isLoading = true;
    });

    final output = await _lyn2vecShellService.showHelp();

    setState(() {
      licenseOutput = output;
      isLoading = false;
    });

    // Show a custom, scrollable toast that lasts indefinitely
    showDialog(
      context: context,
      builder: (context) {
        return AlertDialog(
          title: const Text('Lyn2vec Help Information'),
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

  Future<void> _generateFingerPrintsFile(bool isFirstFile) async {
    String outputDirectory = await _directoryService.getCommonDirectoryPath();

    String step3Directory = _directoryService.step3Directory;

    /// Final Step Directory
    String stepDirectory = '$outputDirectory$step3Directory';

    // Verifica e crea la directory di destinazione, se necessario
    await _directoryService.createStepDirectory('step3_lyn2vec');

    String inputFastaFile = isFirstFile ? filePath1! : filePath2!;

    inputFastaFile = inputFastaFile.split('/').last;

    // Parametri per il comando lyn2vec
    String typeOption =
        selectedOperation; // Selezione dell'operazione (es. 'basic')

    String pathDirectoryOption = outputDirectory; // La directory principale

    if (nValue < 0 || nValue == 0) {
      // Mostra un toast per confermare la generazione del file
      showToast(
        "nValue non può essere zero o minore di zero",
        duration: const Duration(seconds: 2),
        position: ToastPosition.bottom,
        backgroundColor: Colors.red,
        textStyle: const TextStyle(color: Colors.white),
      );
      return;
    }

    String typeFactorizationOption =
        selectedFactorization; // Tipo di fattorizzazione

    String nValueOption = nValue.toString(); // Valore di 'n'
    String revCombOption = revComb ? 'true' : 'false'; // RevComb come booleano
    String outputPathDirectoryOption = stepDirectory; // Percorso di output

    // Esegui il comando per generare i fingerprint
    String result = await _lyn2vecShellService.generateFingerPrints(
      typeOption,
      pathDirectoryOption,
      inputFastaFile,
      typeFactorizationOption,
      nValueOption,
      revCombOption,
      outputPathDirectoryOption,
    );

    // Determina il nome dinamico del file in base al tipo di fattorizzazione
    String generatedFileName =
        '${inputFastaFile.replaceAll('.fasta', '')}-$typeFactorizationOption.txt';

    // Aggiorna il percorso del file generato
    setState(() {
      if (isFirstFile) {
        h5FilePath1 =
            '$stepDirectory/$generatedFileName'; // Nome dinamico del file
      } else {
        h5FilePath2 =
            '$stepDirectory/$generatedFileName'; // Nome dinamico del file
      }
    });

    // Mostra un toast per confermare la generazione del file
    showToast(
      "File generated successfully: $generatedFileName at $stepDirectory",
      duration: const Duration(seconds: 2),
      position: ToastPosition.bottom,
      backgroundColor: Colors.green,
      textStyle: const TextStyle(color: Colors.white),
    );
  }

  bool canProceedToNextStep = true;

  // Funzione per verificare che i file esistano nella directory
  Future<bool> _checkFilesExist() async {
    // Verifica che entrambi i file siano validi e che esistano
    if (h5FilePath1 != null && h5FilePath2 != null) {
      File file1 = File(h5FilePath1!);
      File file2 = File(h5FilePath2!);

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

  // Funzione per procedere al 4° step
  Future<void> _goToNextStep() async {
    // Crea la directory per il 4° step
    await _directoryService
        .createStepDirectory(_directoryService.step4Directory);
    // Naviga al passo successivo
    Navigator.pushNamed(context, '/step4');
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Step 3 - Lyn2vec Fingerprints'),
        actions: [
          IconButton(
            icon: const Icon(Icons.help_outline),
            onPressed: _showLyn2vecHelp,
            tooltip: 'Show Lyn2vec Help',
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
                'Lyn2vec Upload File and Generate FingerPrints!',
                style: TextStyle(fontSize: 18.0, fontWeight: FontWeight.bold),
              ),
              const Text(
                'Upload the files splitted from Step 2. For each file generate a fingerprint file',
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
                            message:
                                'Genera il file Fingerprints dalla sequenza genetica 1',
                            child: ElevatedButton(
                              onPressed: () {
                                _generateFingerPrintsFile(true);
                              },
                              child: const Text('Generate Fingerprints File'),
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
                          ],
                        ],
                      ],
                    ),
                  ),
                  // Divider Width
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
                            message:
                                'Genera il file Fingerprints dalla sequenza genetica 2',
                            child: ElevatedButton(
                              onPressed: () {
                                _generateFingerPrintsFile(false);
                              },
                              child: const Text('Generate Fingerprints File'),
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
                          ],
                        ],
                      ],
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 30),
              const Divider(),
              const Text(
                'Lyn2vec Parameters',
                style: TextStyle(fontSize: 18.0, fontWeight: FontWeight.bold),
              ),
              const SizedBox(height: 10),
              DropdownButtonFormField<String>(
                value: selectedOperation,
                onChanged: (value) {
                  setState(() {
                    selectedOperation = value!;
                  });
                },
                items: lyn2vecOperations.map((String operation) {
                  return DropdownMenuItem<String>(
                    value: operation,
                    child: Text(operation),
                  );
                }).toList(),
                decoration: const InputDecoration(
                  labelText: 'Lyn2vec Operation',
                  border: OutlineInputBorder(),
                ),
              ),
              const SizedBox(height: 10),
              SwitchListTile(
                title: const Text('Rev_Comb'),
                value: revComb,
                onChanged: (value) {
                  setState(() {
                    revComb = value;
                  });
                },
              ),
              const SizedBox(height: 10),
              DropdownButtonFormField<String>(
                value: selectedFactorization,
                onChanged: (value) {
                  setState(() {
                    selectedFactorization = value!;
                  });
                },
                items: factorizationOptions.map((String factorization) {
                  return DropdownMenuItem<String>(
                    value: factorization,
                    child: Text(factorization),
                  );
                }).toList(),
                decoration: const InputDecoration(
                  labelText: 'Type of Factorization',
                  border: OutlineInputBorder(),
                ),
              ),
              const SizedBox(height: 10),
              TextFormField(
                initialValue: nValue.toString(),
                decoration: const InputDecoration(
                  labelText:
                      'N Value (Optional) Inserisci il numero di Sequenze Genetiche contenute all\' interno del file.',
                  border: OutlineInputBorder(),
                ),
                keyboardType: TextInputType.number,
                onChanged: (value) {
                  setState(() {
                    nValue = int.tryParse(value) ?? 1;
                  });
                },
              ),
              const SizedBox(height: 20),
              const Divider(),
              // Button per verificare i file e procedere al 4° step
              ElevatedButton(
                onPressed: canProceedToNextStep
                    ? () async {
                        if (await _checkFilesExist()) {
                          await _goToNextStep();
                        } else {
                          
                          showToast(
                            "Non puoi Procedere al prossimo step. Genera i file delle FingerPrint!",
                            duration: const Duration(seconds: 2),
                            position: ToastPosition.bottom,
                            backgroundColor: Colors.red,
                            textStyle: const TextStyle(color: Colors.white),
                          );

                        }
                      }
                    : null,
                child: const Text('Proceed to Step 4'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

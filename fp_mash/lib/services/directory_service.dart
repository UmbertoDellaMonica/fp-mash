import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:path_provider/path_provider.dart';
import 'dart:io';
import 'package:oktoast/oktoast.dart';

class DirectoryService {

  final String step1Directory = "step1_dsk";
  final String step2Directory = "step2_find_common_kmers";
  final String step3Directory = "step3_lyn2vec";
  final String step4Directory = "step4_mash_sketch";



  // Funzione per ottenere il percorso comune
  Future<String> getCommonDirectoryPath() async {
    // Ottieni la directory dei documenti comuni
    Directory appDocDirectory = await getApplicationDocumentsDirectory();

    // Crea il percorso completo desiderato, che è compatibile con tutte le piattaforme
    String commonDirectoryPath = '${appDocDirectory.path}/fp-mash-software/training/';

    // Restituisci il percorso comune
    return commonDirectoryPath;
  }

  // Funzione per creare una directory per ogni step del processo
  Future<void> createStepDirectory(String stepName) async {
    String baseDirPath = await getCommonDirectoryPath();
    String stepDirectoryPath = '$baseDirPath$stepName/';

    // Verifica se la directory esiste già
    Directory stepDirectory = Directory(stepDirectoryPath);
    if (kDebugMode) {
        print('Directory : $stepDirectory.');
      }
    bool exists = await stepDirectory.exists();

    if (exists) {
      // La directory esiste già, nessuna creazione necessaria
      if (kDebugMode) {
        print('Directory $stepName già esistente.');
      }
    } else {
      // La directory non esiste, quindi la creiamo
      await stepDirectory.create(recursive: true);
      showToast(
          "Directory : $stepDirectory creata con Successo!",
          duration: const Duration(seconds: 3),
          position: ToastPosition.bottom,
          backgroundColor: Colors.green,
          textStyle: const TextStyle(color: Colors.white),
        );
    }
  }

  // Funzione per verificare se una directory esiste
  Future<bool> checkDirectoryExists(String stepName) async {
    String baseDirPath = await getCommonDirectoryPath();
    String stepDirectoryPath = '$baseDirPath$stepName/';

    Directory stepDirectory = Directory(stepDirectoryPath);
    bool exists = await stepDirectory.exists();
    
    return exists;
  }
}

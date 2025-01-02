import 'dart:io';
import 'package:process_run/shell.dart';

class MashShellService {
  final String operation = "mash";

  final String commandSketch = "sketch";
  final String commandDist = "dist";

  final String outputPathFlag = "-o";
  final String fingerPrintFlag = "-fp";
  final String fingerPrintSortedFlag = "-ms";

  // Metodo per mostrare la licenza di mash
  Future<String> showLicense() async {
    final shell = Shell();
    try {
      // Esegui il comando `mash --license`
      final result = await shell.run('mash --license');

      // Se l'esecuzione ha successo, unisci e restituisci l'output
      if (result.isNotEmpty) {
        result.join('\n');
        ProcessResult procText = result.single;
        String text = procText.stdout.toString();
        return text;
      } else {
        return 'No output from mash --license';
      }
    } catch (e) {
      // Gestione degli errori
      return 'Error executing mash --license: $e';
    }
  }

  // Metodo per generare gli sketch fingerprint
  Future<String> generateSketchFingerPrints(
      String inputFilePath, String outputFilePath) async {
    final shell = Shell();
    try {
      // Esegui il comando `mash sketch -fp -ms <inputFilePath> -o <outputFilePath>`
      final result = await shell.run('''
        mash $commandSketch $fingerPrintFlag $fingerPrintSortedFlag $inputFilePath $outputPathFlag $outputFilePath
      ''');

      // Se l'esecuzione ha successo, unisci e restituisci l'output
      if (result.isNotEmpty) {
        result.join('\n');
        ProcessResult procText = result.single;
        String text = procText.stdout.toString();
        return text;
      } else {
        return 'No output from mash sketch command';
      }
    } catch (e) {
      // Gestione degli errori
      return 'Error executing mash sketch command: $e';
    }
  }

  Future<String> calculateDistance(
      String inputFilePath1, String inputFilePath2) async {
    final shell = Shell();
    try {
      // Esegui il comando `mash sketch -fp -ms <inputFilePath> -o <outputFilePath>`
      final result = await shell.run('''
        mash $commandDist $fingerPrintFlag $fingerPrintSortedFlag $inputFilePath1 $inputFilePath2
      ''');

      // Se l'esecuzione ha successo, unisci e restituisci l'output
      if (result.isNotEmpty) {
        result.join('\n');
        ProcessResult procText = result.single;
        String text = procText.stdout.toString();
        return text;
      } else {
        return 'No output from mash sketch command';
      }
    } catch (e) {
      // Gestione degli errori
      return 'Error executing mash sketch command: $e';
    }
  }
}

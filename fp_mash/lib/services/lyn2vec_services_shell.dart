import 'dart:io';

import 'package:process_run/shell.dart';

class Lyn2vecShellService {
  /// Command
  final String command = "lyn2vec";

  /// Help command
  final String helpFlag = "-h";

  /// Generate Fingerprints
  final String typeFlag = "--type";
  final String pathDirectoryFlag = "--path";
  final String inputFileFastaFlag = "--fasta";
  final String typeFactorizationFlag = "--type_factorization";
  final String nValueFlag = "-n";
  final String revCombFlag = "--rev_comb";
  final String outputPathDirectoryFlag = "--output_path";


  

  /// Metodo per mostrare l'help di Lyn2vec
  Future<String> showHelp() async {
    final shell = Shell();
    try {
      // Esegui il comando `mash --license`
      final result = await shell.run('$command $helpFlag');

      // Se l'esecuzione ha successo, unisci e restituisci l'output
      if (result.isNotEmpty) {
        result.join('\n');
        ProcessResult procText = result.single;
        String text = procText.stdout.toString();
        return text;
      } else {
        return 'No output from lyn2vec help';
      }
    } catch (e) {
      // Gestione degli errori
      return 'Error executing lyn2vec : $e';
    }
  }

  Future<String> generateFingerPrints(
      String typeOption,
      String pathDirectoryOption,
      String inputFileFastaOption,
      String typeFactorizationOption,
      String nValueOption,
      String revCombOption,
      String outputPathDirectoryOption
    ) async {
    final shell = Shell();
    try {

      String lyn2vecGenerateFingerprints = """ $command  
      
      $typeFlag $typeOption  
      $pathDirectoryFlag $pathDirectoryOption   
      $inputFileFastaFlag $inputFileFastaOption  
      $outputPathDirectoryFlag $outputPathDirectoryOption
      $typeFactorizationFlag $typeFactorizationOption 
      $nValueFlag $nValueOption
      $revCombFlag $revCombOption
      
      """;
      // Esegui il comando `mash --license`
      final result = await shell.run(lyn2vecGenerateFingerprints);

      // Se l'esecuzione ha successo, unisci e restituisci l'output
      if (result.isNotEmpty) {
        result.join('\n');
        ProcessResult procText = result.single;
        String text = procText.stdout.toString();
        return text;
      } else {
        return 'No output from lyn2vec help';
      }
    } catch (e) {
      // Gestione degli errori
      return 'Error executing lyn2vec : $e';
    }
  }
}

import 'dart:io';
import 'package:process_run/shell.dart';

class DskShellService {

  final String dskOperation = "dsk";
  final String dskConversionOperation = "dsk2ascii";

  final String fileFlag = "-file";
  final String outputFileName = "-out";
  final String outputDirectoryFile = "-out-dir";
  final String help = "-help";

  final String fastaConversion = "-fasta";


  final Shell shell = Shell();

  // Metodo per eseguire 'dsk -help'
  Future<String> showHelp() async {
    try {
      // Esegui il comando `dsk -help`
      final result = await shell.run('$dskOperation $help');

      // Se l'esecuzione ha successo, unisci e restituisci l'output
      if (result.isNotEmpty) {
        //result.join('\n');
        ProcessResult procText = result.single;
        String text = procText.stdout.toString();
        return text;
      } else {
        return 'No output from dsk -help';
      }
    } catch (e) {
      // Gestione degli errori
      return 'Error executing dsk -help: $e';
    }
  }

  Future<String> generateH5File(
      String inputFilePath, String outputDirectory) async {
    try {
      final result = await shell.run('$dskOperation $fileFlag $inputFilePath $outputDirectoryFile $outputDirectory');

      // Controlla se il comando è stato eseguito correttamente
      if (result.isNotEmpty) {
        //result.join('\n');
        ProcessResult procText = result.single;
        String text = procText.stdout.toString();
        return text; // Restituisci l'output del comando
      } else {
        return 'Error executing dsk ';
      }
    } catch (e) {
      return 'Exception: $e'; // Restituisci l'errore in caso di eccezione
    }
  }


   // Metodo per convertire il file HDF5 in formato FASTA
  Future<String> convertH5ToFasta(String h5FilePath, String outputFastaFile) async {
    try {
      // Esegui il comando dsk2ascii per la conversione
      final result = await shell.run('$dskConversionOperation $fileFlag $h5FilePath $fastaConversion  $outputFileName $outputFastaFile');

      // Controlla se il comando è stato eseguito correttamente
      if (result.isNotEmpty) {
        ProcessResult procText = result.single;
        String text = procText.stdout.toString();
        return text; // Restituisci l'output della conversione
      } else {
        return 'Error executing dsk2ascii';
      }
    } catch (e) {
      return 'Exception: $e'; // Restituisci l'errore in caso di eccezione
    }
  }
}

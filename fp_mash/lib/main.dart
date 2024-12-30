import 'package:flutter/material.dart';
import 'package:fp_mash/screen/steps/step_2_screen.dart';
import 'package:fp_mash/screen/steps/step_3_screen.dart';
import 'package:oktoast/oktoast.dart';

import 'screen/home/home_screen.dart';
import 'screen/steps/step_1_screen.dart';

void main() {
  runApp(
    OKToast(child:
  
  MyApp())
  );
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'fp-mash Software',
      theme: ThemeData(
        // This is the theme of your application.
        //
        // TRY THIS: Try running your application with "flutter run". You'll see
        // the application has a purple toolbar. Then, without quitting the app,
        // try changing the seedColor in the colorScheme below to Colors.green
        // and then invoke "hot reload" (save your changes or press the "hot
        // reload" button in a Flutter-supported IDE, or press "r" if you used
        // the command line to start the app).
        //
        // Notice that the counter didn't reset back to zero; the application
        // state is not lost during the reload. To reset the state, use hot
        // restart instead.
        //
        // This works for code too, not just values: Most code changes can be
        // tested with just a hot reload.
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'fp-mash'),
      routes: {
        '/step1': (context) => Step1Screen(),
        '/step2': (context) => Step2Screen(),
        '/step3': (context) => Step3Screen(),
      },
    );
  }
}

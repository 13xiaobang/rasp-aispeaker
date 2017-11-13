package test.ai.speaker.commands;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;


import com.android.internal.os.BaseCommand;
import java.io.PrintStream;
import ai.speaker.AiSpeakerManager;

public class TestTmCmd extends BaseCommand {
    public static void main(String[] args) {
        (new TestTmCmd()).run(args);
        return;
    }
    public void onRun() throws Exception {
        String op = nextArgRequired();
/*
        if (op.equals("setmode"))
        {
            String id = nextArg();
            if (id == null)
            {
                onShowUsage(System.err);
                return;
            }
            AiSpeakerManager.getInstance().setMode(Integer.parseInt(id));
            System.out.println("TestTmCmd: setmode: " + Integer.parseInt(id));
        }
		else if(op.equals("getmode"))
		{
			int  mode  = AiSpeakerManager.getInstance().getMode();
			System.out.println("TestTmCmd: getMode: " + mode);
		}
        else if(op.equals("pollevent"))
        {
            int temp = AiSpeakerManager.getInstance().pollEvent();
            System.out.println("TestTmCmd: pollEvent: " + temp);
			String  str = AiSpeakerManager.getInstance().getStr();
			System.out.println("TestTmCmd: getStr: " + str);
        }
		else if(op.equals("getstr"))
		{
			String  str = AiSpeakerManager.getInstance().getStr();
			System.out.println("TestTmCmd: getStr: " + str);
		}
		else if(op.equals("getpcm"))
		{
			byte bt[] = AiSpeakerManager.getInstance().getPcm();
	        try {
				File file = new File("/data", "respeaker.pcm");
				try {
		            file.createNewFile();
		        } catch (IOException e) {
		            // TODO Auto-generated catch block
		            e.printStackTrace();
		        }
	            FileOutputStream in = new FileOutputStream(file);
	            try {
	                in.write(bt, 0, bt.length);
	                in.close();
	                // boolean success=true;  
	                System.out.println("write file done");
	            } catch (IOException e) {
	                // TODO Auto-generated catch block
	                e.printStackTrace();
	            }
	        } catch (FileNotFoundException e) {
	            // TODO Auto-generated catch block
	            e.printStackTrace();
	        }
				System.out.println("TestTmCmd: getPcm:  done ");
				AiSpeakerManager.getInstance().setMode(0);
            	System.out.println("TestTmCmd: setmode: 0 done.");
			}

        else {
            showError("Error: unknow command '" + op + "'");
        }

*/
        return;
    }

    public void onShowUsage(PrintStream out) {
        out.println(
                 "help:\n" +
                 "setmode    [0/1]\n" +
                 "getmode\n"
                );
    }
}
